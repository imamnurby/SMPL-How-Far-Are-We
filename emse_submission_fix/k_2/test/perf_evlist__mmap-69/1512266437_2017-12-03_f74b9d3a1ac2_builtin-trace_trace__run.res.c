static int trace__run(struct trace *trace, int argc, const char **argv) {
  struct perf_evlist *evlist = trace->evlist;
  struct perf_evsel *evsel, *pgfault_maj = NULL, *pgfault_min = NULL;
  int err = -1, i;
  unsigned long before;
  const bool forks = argc > 0;
  bool draining = false;
  trace->live = true;
  if (trace->trace_syscalls && trace__add_syscall_newtp(trace)) goto out_error_raw_syscalls;
  if (trace->trace_syscalls) trace->vfs_getname = perf_evlist__add_vfs_getname(evlist);
  if ((trace->trace_pgfaults & TRACE_PFMAJ)) {
    pgfault_maj = perf_evsel__new_pgfault(PERF_COUNT_SW_PAGE_FAULTS_MAJ);
    if (pgfault_maj == NULL) goto out_error_mem;
    perf_evlist__add(evlist, pgfault_maj);
  }
  if ((trace->trace_pgfaults & TRACE_PFMIN)) {
    pgfault_min = perf_evsel__new_pgfault(PERF_COUNT_SW_PAGE_FAULTS_MIN);
    if (pgfault_min == NULL) goto out_error_mem;
    perf_evlist__add(evlist, pgfault_min);
  }
  if (trace->sched && perf_evlist__add_newtp(evlist, "sched", "sched_stat_runtime", trace__sched_stat_runtime)) goto out_error_sched_stat_runtime;
  err = perf_evlist__create_maps(evlist, &trace->opts.target);
  if (err < 0) {
    fprintf(trace->output, "Problems parsing the target to trace, check your options!\n");
    goto out_delete_evlist;
  }
  err = trace__symbols_init(trace, evlist);
  if (err < 0) {
    fprintf(trace->output, "Problems initializing symbol libraries!\n");
    goto out_delete_evlist;
  }
  perf_evlist__config(evlist, &trace->opts, NULL);
  if (callchain_param.enabled) {
    bool use_identifier = false;
    if (trace->syscalls.events.sys_exit) {
      perf_evsel__config_callchain(trace->syscalls.events.sys_exit, &trace->opts, &callchain_param);
      use_identifier = true;
    }
    if (pgfault_maj) {
      perf_evsel__config_callchain(pgfault_maj, &trace->opts, &callchain_param);
      use_identifier = true;
    }
    if (pgfault_min) {
      perf_evsel__config_callchain(pgfault_min, &trace->opts, &callchain_param);
      use_identifier = true;
    }
    if (use_identifier) {
      /*
       * Now we have evsels with different sample_ids, use
       * PERF_SAMPLE_IDENTIFIER to map from sample to evsel
       * from a fixed position in each ring buffer record.
       *
       * As of this the changeset introducing this comment, this
       * isn't strictly needed, as the fields that can come before
       * PERF_SAMPLE_ID are all used, but we'll probably disable
       * some of those for things like copying the payload of
       * pointer syscall arguments, and for vfs_getname we don't
       * need PERF_SAMPLE_ADDR and PERF_SAMPLE_IP, so do this
       * here as a warning we need to use PERF_SAMPLE_IDENTIFIER.
       */
      perf_evlist__set_sample_bit(evlist, IDENTIFIER);
      perf_evlist__reset_sample_bit(evlist, ID);
    }
  }
  signal(SIGCHLD, sig_handler);
  signal(SIGINT, sig_handler);
  if (forks) {
    err = perf_evlist__prepare_workload(evlist, &trace->opts.target, argv, false, NULL);
    if (err < 0) {
      fprintf(trace->output, "Couldn't run the workload!\n");
      goto out_delete_evlist;
    }
  }
  err = perf_evlist__open(evlist);
  if (err < 0) goto out_error_open;
  err = bpf__apply_obj_config();
  if (err) {
    char errbuf[BUFSIZ];
    bpf__strerror_apply_obj_config(err, errbuf, sizeof(errbuf));
    pr_err("ERROR: Apply config to BPF failed: %s\n", errbuf);
    goto out_error_open;
  }
  /*
   * Better not use !target__has_task() here because we need to cover the
   * case where no threads were specified in the command line, but a
   * workload was, and in that case we will fill in the thread_map when
   * we fork the workload in perf_evlist__prepare_workload.
   */
  if (trace->filter_pids.nr > 0)
    err = perf_evlist__set_filter_pids(evlist, trace->filter_pids.nr, trace->filter_pids.entries);
  else if (thread_map__pid(evlist->threads, 0) == -1)
    err = trace__set_filter_loop_pids(trace);
  if (err < 0) goto out_error_mem;
  if (trace->ev_qualifier_ids.nr > 0) {
    err = trace__set_ev_qualifier_filter(trace);
    if (err < 0) goto out_errno;
    pr_debug("event qualifier tracepoint filter: %s\n", trace->syscalls.events.sys_exit->filter);
  }
  err = perf_evlist__apply_filters(evlist, &evsel);
  if (err < 0) goto out_error_apply_filters;
  err = perf_evlist__mmap(evlist, trace->opts.mmap_pages);
  if (err < 0) goto out_error_mmap;
  if (!target__none(&trace->opts.target) && !trace->opts.initial_delay) perf_evlist__enable(evlist);
  if (forks) perf_evlist__start_workload(evlist);
  if (trace->opts.initial_delay) {
    usleep(trace->opts.initial_delay * 1000);
    perf_evlist__enable(evlist);
  }
  trace->multiple_threads = thread_map__pid(evlist->threads, 0) == -1 || evlist->threads->nr > 1 || perf_evlist__first(evlist)->attr.inherit;
again:
  before = trace->nr_events;
  for (i = 0; i < evlist->nr_mmaps; i++) {
    union perf_event *event;
    while ((event = perf_evlist__mmap_read(evlist, i)) != NULL) {
      struct perf_sample sample;
      ++trace->nr_events;
      err = perf_evlist__parse_sample(evlist, event, &sample);
      if (err) {
        fprintf(trace->output, "Can't parse sample, err = %d, skipping...\n", err);
        goto next_event;
      }
      trace__handle_event(trace, event, &sample);
    next_event:
      perf_evlist__mmap_consume(evlist, i);
      if (interrupted) goto out_disable;
      if (done && !draining) {
        perf_evlist__disable(evlist);
        draining = true;
      }
    }
  }
  if (trace->nr_events == before) {
    int timeout = done ? 100 : -1;
    if (!draining && perf_evlist__poll(evlist, timeout) > 0) {
      if (perf_evlist__filter_pollfd(evlist, POLLERR | POLLHUP) == 0) draining = true;
      goto again;
    }
  } else {
    goto again;
  }
out_disable:
  thread__zput(trace->current);
  perf_evlist__disable(evlist);
  if (!err) {
    if (trace->summary) trace__fprintf_thread_summary(trace, trace->output);
    if (trace->show_tool_stats) {
      fprintf(trace->output,
              "Stats:\n "
              " vfs_getname : %" PRIu64 "\n"
              " proc_getname: %" PRIu64 "\n",
              trace->stats.vfs_getname, trace->stats.proc_getname);
    }
  }
out_delete_evlist:
  trace__symbols__exit(trace);
  perf_evlist__delete(evlist);
  trace->evlist = NULL;
  trace->live = false;
  return err;
  {
    char errbuf[BUFSIZ];
  out_error_sched_stat_runtime:
    tracing_path__strerror_open_tp(errno, errbuf, sizeof(errbuf), "sched", "sched_stat_runtime");
    goto out_error;
  out_error_raw_syscalls:
    tracing_path__strerror_open_tp(errno, errbuf, sizeof(errbuf), "raw_syscalls", "sys_(enter|exit)");
    goto out_error;
  out_error_mmap:
    perf_evlist__strerror_mmap(evlist, errno, errbuf, sizeof(errbuf));
    goto out_error;
  out_error_open:
    perf_evlist__strerror_open(evlist, errno, errbuf, sizeof(errbuf));
  out_error:
    fprintf(trace->output, "%s\n", errbuf);
    goto out_delete_evlist;
  out_error_apply_filters:
    fprintf(trace->output, "Failed to set filter \"%s\" on event %s with %d (%s)\n", evsel->filter, perf_evsel__name(evsel), errno, str_error_r(errno, errbuf, sizeof(errbuf)));
    goto out_delete_evlist;
  }
out_error_mem:
  fprintf(trace->output, "Not enough memory to run!\n");
  goto out_delete_evlist;
out_errno:
  fprintf(trace->output, "errno=%d,%s\n", errno, strerror(errno));
  goto out_delete_evlist;
}