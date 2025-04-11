static int do_test(struct bpf_object *obj, int (*func)(void), int expect) {
  struct record_opts opts = {
      .target =
          {
              .uid = UINT_MAX,
              .uses_mmap = true,
          },
      .freq = 0,
      .mmap_pages = 256,
      .default_interval = 1,
  };
  char pid[16];
  char sbuf[STRERR_BUFSIZE];
  struct perf_evlist *evlist;
  int i, ret = TEST_FAIL, err = 0, count = 0;
  struct parse_events_state parse_state;
  struct parse_events_error parse_error;
  bzero(&parse_error, sizeof(parse_error));
  bzero(&parse_state, sizeof(parse_state));
  parse_state.error = &parse_error;
  INIT_LIST_HEAD(&parse_state.list);
  err = parse_events_load_bpf_obj(&parse_state, &parse_state.list, obj, NULL);
  if (err || list_empty(&parse_state.list)) {
    pr_debug("Failed to add events selected by BPF\n");
    return TEST_FAIL;
  }
  snprintf(pid, sizeof(pid), "%d", getpid());
  pid[sizeof(pid) - 1] = '\0';
  opts.target.tid = opts.target.pid = pid;
  /* Instead of perf_evlist__new_default, don't add default events */
  evlist = perf_evlist__new();
  if (!evlist) {
    pr_debug("Not enough memory to create evlist\n");
    return TEST_FAIL;
  }
  err = perf_evlist__create_maps(evlist, &opts.target);
  if (err < 0) {
    pr_debug("Not enough memory to create thread/cpu maps\n");
    goto out_delete_evlist;
  }
  perf_evlist__splice_list_tail(evlist, &parse_state.list);
  evlist->nr_groups = parse_state.nr_groups;
  perf_evlist__config(evlist, &opts, NULL);
  err = perf_evlist__open(evlist);
  if (err < 0) {
    pr_debug("perf_evlist__open: %s\n", str_error_r(errno, sbuf, sizeof(sbuf)));
    goto out_delete_evlist;
  }
  err = perf_evlist__mmap(evlist, opts.mmap_pages);
  if (err < 0) {
    pr_debug("perf_evlist__mmap: %s\n", str_error_r(errno, sbuf, sizeof(sbuf)));
    goto out_delete_evlist;
  }
  perf_evlist__enable(evlist);
  (*func)();
  perf_evlist__disable(evlist);
  for (i = 0; i < evlist->nr_mmaps; i++) {
    union perf_event *event;
    while ((event = perf_evlist__mmap_read(evlist, i)) != NULL) {
      const u32 type = event->header.type;
      if (type == PERF_RECORD_SAMPLE) count++;
    }
  }
  if (count != expect) {
    pr_debug("BPF filter result incorrect\n");
    goto out_delete_evlist;
  }
  ret = TEST_OK;
out_delete_evlist:
  perf_evlist__delete(evlist);
  return ret;
}