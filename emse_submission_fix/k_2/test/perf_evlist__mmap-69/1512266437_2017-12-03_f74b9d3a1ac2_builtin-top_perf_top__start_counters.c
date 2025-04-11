static int perf_top__start_counters(struct perf_top *top) {
  char msg[BUFSIZ];
  struct perf_evsel *counter;
  struct perf_evlist *evlist = top->evlist;
  struct record_opts *opts = &top->record_opts;
  perf_evlist__config(evlist, opts, &callchain_param);
  evlist__for_each_entry(evlist, counter) {
  try_again:
    if (perf_evsel__open(counter, top->evlist->cpus, top->evlist->threads) < 0) {
      if (perf_evsel__fallback(counter, errno, msg, sizeof(msg))) {
        if (verbose > 0) ui__warning("%s\n", msg);
        goto try_again;
      }
      perf_evsel__open_strerror(counter, &opts->target, errno, msg, sizeof(msg));
      ui__error("%s\n", msg);
      goto out_err;
    }
  }
  if (perf_evlist__mmap(evlist, opts->mmap_pages, false) < 0) {
    ui__error("Failed to mmap with %d (%s)\n", errno, str_error_r(errno, msg, sizeof(msg)));
    goto out_err;
  }
  return 0;
out_err:
  return -1;
}