int test__task_exit(struct test *test __maybe_unused, int subtest __maybe_unused)
{
  int err = -1;
  union perf_event *event;
  struct perf_evsel *evsel;
  struct perf_evlist *evlist;
  struct target target = {
      .uid = UINT_MAX,
      .uses_mmap = true,
  };
  const char *argv[] = {"true", NULL};
  char sbuf[STRERR_BUFSIZE];
  struct cpu_map *cpus;
  struct thread_map *threads;
  signal(SIGCHLD, sig_handler);
  evlist = perf_evlist__new_default();
  if (evlist == NULL)
    {
      pr_debug("perf_evlist__new_default\n");
      return -1;
    }

  cpus = cpu_map__dummy_new();
  threads = thread_map__new_by_tid(-1);
  if (!cpus || !threads)
    {
      err = -ENOMEM;
      pr_debug("Not enough memory to create thread/cpu maps\n");
      goto out_free_maps;
    }
  perf_evlist__set_maps(evlist, cpus, threads);
  cpus = NULL;
  threads = NULL;
  err = perf_evlist__prepare_workload(evlist, &target, argv, false, workload_exec_failed_signal);
  if (err < 0)
    {
      pr_debug("Couldn't run the workload!\n");
      goto out_delete_evlist;
    }
  evsel = perf_evlist__first(evlist);
  evsel->attr.task = 1;
#ifdef __s390x__
  evsel->attr.sample_freq = 1000000;
#else
  evsel->attr.sample_freq = 1;
#endif
  evsel->attr.inherit = 0;
  evsel->attr.watermark = 0;
  evsel->attr.wakeup_events = 1;
  evsel->attr.exclude_kernel = 1;
  err = perf_evlist__open(evlist);
  if (err < 0)
    {
      pr_debug("Couldn't open the evlist: %s\n", str_error_r(-err, sbuf, sizeof(sbuf)));
      goto out_delete_evlist;
    }
  if (perf_evlist__mmap(evlist, 128) < 0)
    {
      pr_debug("failed to mmap events: %d (%s)\n", errno, str_error_r(errno, sbuf, sizeof(sbuf)));
      goto out_delete_evlist;
    }
  perf_evlist__start_workload(evlist);
retry:
  while ((event = perf_evlist__mmap_read(evlist, 0)) != NULL)
    {
      if (event->header.type == PERF_RECORD_EXIT)
        nr_exit++;
      perf_evlist__mmap_consume(evlist, 0);
    }
  if (!exited || !nr_exit)
    {
      perf_evlist__poll(evlist, -1);
      goto retry;
    }
  if (nr_exit != 1)
    {
      pr_debug("received %d EXIT records\n", nr_exit);
      err = -1;
    }
out_free_maps:
  cpu_map__put(cpus);
  thread_map__put(threads);
out_delete_evlist:
  perf_evlist__delete(evlist);
  return err;
}