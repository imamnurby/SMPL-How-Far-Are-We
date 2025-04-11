int test__perf_time_to_tsc(struct test *test __maybe_unused, int subtest __maybe_unused) {
  struct record_opts opts = {
      .mmap_pages = UINT_MAX,
      .user_freq = UINT_MAX,
      .user_interval = ULLONG_MAX,
      .target =
          {
              .uses_mmap = true,
          },
      .sample_time = true,
  };
  struct thread_map *threads = NULL;
  struct cpu_map *cpus = NULL;
  struct perf_evlist *evlist = NULL;
  struct perf_evsel *evsel = NULL;
  int err = -1, ret, i;
  const char *comm1, *comm2;
  struct perf_tsc_conversion tc;
  struct perf_event_mmap_page *pc;
  union perf_event *event;
  u64 test_tsc, comm1_tsc, comm2_tsc;
  u64 test_time, comm1_time = 0, comm2_time = 0;
  threads = thread_map__new(-1, getpid(), UINT_MAX);
  CHECK_NOT_NULL__(threads);
  cpus = cpu_map__new(NULL);
  CHECK_NOT_NULL__(cpus);
  evlist = perf_evlist__new();
  CHECK_NOT_NULL__(evlist);
  perf_evlist__set_maps(evlist, cpus, threads);
  CHECK__(parse_events(evlist, "cycles:u", NULL));
  perf_evlist__config(evlist, &opts, NULL);
  evsel = perf_evlist__first(evlist);
  evsel->attr.comm = 1;
  evsel->attr.disabled = 1;
  evsel->attr.enable_on_exec = 0;
  CHECK__(perf_evlist__open(evlist));
  CHECK__(perf_evlist__mmap(evlist, UINT_MAX));
  pc = evlist->mmap[0].base;
  ret = perf_read_tsc_conversion(pc, &tc);
  if (ret) {
    if (ret == -EOPNOTSUPP) {
      fprintf(stderr, " (not supported)");
      return 0;
    }
    goto out_err;
  }
  perf_evlist__enable(evlist);
  comm1 = "Test COMM 1";
  CHECK__(prctl(PR_SET_NAME, (unsigned long)comm1, 0, 0, 0));
  test_tsc = rdtsc();
  comm2 = "Test COMM 2";
  CHECK__(prctl(PR_SET_NAME, (unsigned long)comm2, 0, 0, 0));
  perf_evlist__disable(evlist);
  for (i = 0; i < evlist->nr_mmaps; i++) {
    while ((event = perf_evlist__mmap_read(evlist, i)) != NULL) {
      struct perf_sample sample;
      if (event->header.type != PERF_RECORD_COMM || (pid_t)event->comm.pid != getpid() || (pid_t)event->comm.tid != getpid()) goto next_event;
      if (strcmp(event->comm.comm, comm1) == 0) {
        CHECK__(perf_evsel__parse_sample(evsel, event, &sample));
        comm1_time = sample.time;
      }
      if (strcmp(event->comm.comm, comm2) == 0) {
        CHECK__(perf_evsel__parse_sample(evsel, event, &sample));
        comm2_time = sample.time;
      }
    next_event:
      perf_evlist__mmap_consume(evlist, i);
    }
  }
  if (!comm1_time || !comm2_time) goto out_err;
  test_time = tsc_to_perf_time(test_tsc, &tc);
  comm1_tsc = perf_time_to_tsc(comm1_time, &tc);
  comm2_tsc = perf_time_to_tsc(comm2_time, &tc);
  pr_debug("1st event perf time %" PRIu64 " tsc %" PRIu64 "\n", comm1_time, comm1_tsc);
  pr_debug("rdtsc          time %" PRIu64 " tsc %" PRIu64 "\n", test_time, test_tsc);
  pr_debug("2nd event perf time %" PRIu64 " tsc %" PRIu64 "\n", comm2_time, comm2_tsc);
  if (test_time <= comm1_time || test_time >= comm2_time) goto out_err;
  if (test_tsc <= comm1_tsc || test_tsc >= comm2_tsc) goto out_err;
  err = 0;
out_err:
  perf_evlist__delete(evlist);
  return err;
}