int test__keep_tracking(struct test *test __maybe_unused, int subtest __maybe_unused)
{
  struct record_opts opts = {
      .mmap_pages = UINT_MAX,
      .user_freq = UINT_MAX,
      .user_interval = ULLONG_MAX,
      .target =
          {
              .uses_mmap = true,
          },
  };
  struct thread_map *threads = NULL;
  struct cpu_map *cpus = NULL;
  struct perf_evlist *evlist = NULL;
  struct perf_evsel *evsel = NULL;
  int found, err = -1;
  const char *comm;
  threads = thread_map__new(-1, getpid(), UINT_MAX);
  CHECK_NOT_NULL__(threads);
  cpus = cpu_map__new(NULL);
  CHECK_NOT_NULL__(cpus);
  evlist = perf_evlist__new();
  CHECK_NOT_NULL__(evlist);
  perf_evlist__set_maps(evlist, cpus, threads);
  CHECK__(parse_events(evlist, "dummy:u", NULL));
  CHECK__(parse_events(evlist, "cycles:u", NULL));
  perf_evlist__config(evlist, &opts, NULL);
  evsel = perf_evlist__first(evlist);
  evsel->attr.comm = 1;
  evsel->attr.disabled = 1;
  evsel->attr.enable_on_exec = 0;
  if (perf_evlist__open(evlist) < 0)
    {
      pr_debug("Unable to open dummy and cycles event\n");
      err = TEST_SKIP;
      goto out_err;
    }
  CHECK__(perf_evlist__mmap(evlist, UINT_MAX, false));

  perf_evlist__enable(evlist);
  comm = "Test COMM 1";
  CHECK__(prctl(PR_SET_NAME, (unsigned long)comm, 0, 0, 0));
  perf_evlist__disable(evlist);
  found = find_comm(evlist, comm);
  if (found != 1)
    {
      pr_debug("First time, failed to find tracking event.\n");
      goto out_err;
    }

  perf_evlist__enable(evlist);
  evsel = perf_evlist__last(evlist);
  CHECK__(perf_evsel__disable(evsel));
  comm = "Test COMM 2";
  CHECK__(prctl(PR_SET_NAME, (unsigned long)comm, 0, 0, 0));
  perf_evlist__disable(evlist);
  found = find_comm(evlist, comm);
  if (found != 1)
    {
      pr_debug("Seconf time, failed to find tracking event.\n");
      goto out_err;
    }
  err = 0;
out_err:
  if (evlist)
    {
      perf_evlist__disable(evlist);
      perf_evlist__delete(evlist);
    }
  else
    {
      cpu_map__put(cpus);
      thread_map__put(threads);
    }
  return err;
}