int test__switch_tracking(struct test *test __maybe_unused, int subtest __maybe_unused)
{
  const char *sched_switch = "sched:sched_switch";
  struct switch_tracking switch_tracking = {
      .tids = NULL,
  };
  struct record_opts opts = {
      .mmap_pages = UINT_MAX,
      .user_freq = UINT_MAX,
      .user_interval = ULLONG_MAX,
      .freq = 4000,
      .target =
          {
              .uses_mmap = true,
          },
  };
  struct thread_map *threads = NULL;
  struct cpu_map *cpus = NULL;
  struct perf_evlist *evlist = NULL;
  struct perf_evsel *evsel, *cpu_clocks_evsel, *cycles_evsel;
  struct perf_evsel *switch_evsel, *tracking_evsel;
  const char *comm;
  int err = -1;
  threads = thread_map__new(-1, getpid(), UINT_MAX);
  if (!threads)
    {
      pr_debug("thread_map__new failed!\n");
      goto out_err;
    }
  cpus = cpu_map__new(NULL);
  if (!cpus)
    {
      pr_debug("cpu_map__new failed!\n");
      goto out_err;
    }
  evlist = perf_evlist__new();
  if (!evlist)
    {
      pr_debug("perf_evlist__new failed!\n");
      goto out_err;
    }
  perf_evlist__set_maps(evlist, cpus, threads);

  err = parse_events(evlist, "cpu-clock:u", NULL);
  if (err)
    {
      pr_debug("Failed to parse event dummy:u\n");
      goto out_err;
    }
  cpu_clocks_evsel = perf_evlist__last(evlist);

  err = parse_events(evlist, "cycles:u", NULL);
  if (err)
    {
      pr_debug("Failed to parse event cycles:u\n");
      goto out_err;
    }
  cycles_evsel = perf_evlist__last(evlist);

  if (!perf_evlist__can_select_event(evlist, sched_switch))
    {
      pr_debug("No sched_switch\n");
      err = 0;
      goto out;
    }
  err = parse_events(evlist, sched_switch, NULL);
  if (err)
    {
      pr_debug("Failed to parse event %s\n", sched_switch);
      goto out_err;
    }
  switch_evsel = perf_evlist__last(evlist);
  perf_evsel__set_sample_bit(switch_evsel, CPU);
  perf_evsel__set_sample_bit(switch_evsel, TIME);
  switch_evsel->system_wide = true;
  switch_evsel->no_aux_samples = true;
  switch_evsel->immediate = true;

  if (cycles_evsel == perf_evlist__first(evlist))
    {
      pr_debug("cycles event already at front");
      goto out_err;
    }
  perf_evlist__to_front(evlist, cycles_evsel);
  if (cycles_evsel != perf_evlist__first(evlist))
    {
      pr_debug("Failed to move cycles event to front");
      goto out_err;
    }
  perf_evsel__set_sample_bit(cycles_evsel, CPU);
  perf_evsel__set_sample_bit(cycles_evsel, TIME);

  err = parse_events(evlist, "dummy:u", NULL);
  if (err)
    {
      pr_debug("Failed to parse event dummy:u\n");
      goto out_err;
    }
  tracking_evsel = perf_evlist__last(evlist);
  perf_evlist__set_tracking_event(evlist, tracking_evsel);
  tracking_evsel->attr.freq = 0;
  tracking_evsel->attr.sample_period = 1;
  perf_evsel__set_sample_bit(tracking_evsel, TIME);

  perf_evlist__config(evlist, &opts, NULL);

  if (cycles_evsel != perf_evlist__first(evlist))
    {
      pr_debug("Front event no longer at front");
      goto out_err;
    }

  if (!tracking_evsel->attr.mmap || !tracking_evsel->attr.comm)
    {
      pr_debug("Tracking event not tracking\n");
      goto out_err;
    }

  evlist__for_each_entry(evlist, evsel)
  {
    if (evsel != tracking_evsel)
      {
        if (evsel->attr.mmap || evsel->attr.comm)
          {
            pr_debug("Non-tracking event is tracking\n");
            goto out_err;
          }
      }
  }
  if (perf_evlist__open(evlist) < 0)
    {
      pr_debug("Not supported\n");
      err = 0;
      goto out;
    }
  err = perf_evlist__mmap(evlist, UINT_MAX, false);
  if (err)
    {
      pr_debug("perf_evlist__mmap failed!\n");
      goto out_err;
    }
  perf_evlist__enable(evlist);
  err = perf_evsel__disable(cpu_clocks_evsel);
  if (err)
    {
      pr_debug("perf_evlist__disable_event failed!\n");
      goto out_err;
    }
  err = spin_sleep();
  if (err)
    {
      pr_debug("spin_sleep failed!\n");
      goto out_err;
    }
  comm = "Test COMM 1";
  err = prctl(PR_SET_NAME, (unsigned long)comm, 0, 0, 0);
  if (err)
    {
      pr_debug("PR_SET_NAME failed!\n");
      goto out_err;
    }
  err = perf_evsel__disable(cycles_evsel);
  if (err)
    {
      pr_debug("perf_evlist__disable_event failed!\n");
      goto out_err;
    }
  comm = "Test COMM 2";
  err = prctl(PR_SET_NAME, (unsigned long)comm, 0, 0, 0);
  if (err)
    {
      pr_debug("PR_SET_NAME failed!\n");
      goto out_err;
    }
  err = spin_sleep();
  if (err)
    {
      pr_debug("spin_sleep failed!\n");
      goto out_err;
    }
  comm = "Test COMM 3";
  err = prctl(PR_SET_NAME, (unsigned long)comm, 0, 0, 0);
  if (err)
    {
      pr_debug("PR_SET_NAME failed!\n");
      goto out_err;
    }
  err = perf_evsel__enable(cycles_evsel);
  if (err)
    {
      pr_debug("perf_evlist__disable_event failed!\n");
      goto out_err;
    }
  comm = "Test COMM 4";
  err = prctl(PR_SET_NAME, (unsigned long)comm, 0, 0, 0);
  if (err)
    {
      pr_debug("PR_SET_NAME failed!\n");
      goto out_err;
    }
  err = spin_sleep();
  if (err)
    {
      pr_debug("spin_sleep failed!\n");
      goto out_err;
    }
  perf_evlist__disable(evlist);
  switch_tracking.switch_evsel = switch_evsel;
  switch_tracking.cycles_evsel = cycles_evsel;
  err = process_events(evlist, &switch_tracking);
  zfree(&switch_tracking.tids);
  if (err)
    goto out_err;

  if (!switch_tracking.comm_seen[0] || !switch_tracking.comm_seen[1] || !switch_tracking.comm_seen[2] || !switch_tracking.comm_seen[3])
    {
      pr_debug("Missing comm events\n");
      goto out_err;
    }

  if (!switch_tracking.cycles_before_comm_1)
    {
      pr_debug("Missing cycles events\n");
      goto out_err;
    }

  if (switch_tracking.cycles_between_comm_2_and_comm_3)
    {
      pr_debug("cycles events even though event was disabled\n");
      goto out_err;
    }

  if (!switch_tracking.cycles_after_comm_4)
    {
      pr_debug("Missing cycles events\n");
      goto out_err;
    }
out:
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
out_err:
  err = -1;
  goto out;
}