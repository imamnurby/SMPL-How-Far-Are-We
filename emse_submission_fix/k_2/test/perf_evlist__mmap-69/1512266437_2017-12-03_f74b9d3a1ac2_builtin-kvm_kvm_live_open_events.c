static int kvm_live_open_events(struct perf_kvm_stat *kvm) {
  int err, rc = -1;
  struct perf_evsel *pos;
  struct perf_evlist *evlist = kvm->evlist;
  char sbuf[STRERR_BUFSIZE];
  perf_evlist__config(evlist, &kvm->opts, NULL);
  /*
   * Note: exclude_{guest,host} do not apply here.
   *       This command processes KVM tracepoints from host only
   */
  evlist__for_each_entry(evlist, pos) {
    struct perf_event_attr *attr = &pos->attr;
    /* make sure these *are* set */
    perf_evsel__set_sample_bit(pos, TID);
    perf_evsel__set_sample_bit(pos, TIME);
    perf_evsel__set_sample_bit(pos, CPU);
    perf_evsel__set_sample_bit(pos, RAW);
    /* make sure these are *not*; want as small a sample as possible */
    perf_evsel__reset_sample_bit(pos, PERIOD);
    perf_evsel__reset_sample_bit(pos, IP);
    perf_evsel__reset_sample_bit(pos, CALLCHAIN);
    perf_evsel__reset_sample_bit(pos, ADDR);
    perf_evsel__reset_sample_bit(pos, READ);
    attr->mmap = 0;
    attr->comm = 0;
    attr->task = 0;
    attr->sample_period = 1;
    attr->watermark = 0;
    attr->wakeup_events = 1000;
    /* will enable all once we are ready */
    attr->disabled = 1;
  }
  err = perf_evlist__open(evlist);
  if (err < 0) {
    printf("Couldn't create the events: %s\n", str_error_r(errno, sbuf, sizeof(sbuf)));
    goto out;
  }
  if (perf_evlist__mmap(evlist, kvm->opts.mmap_pages, false) < 0) {
    ui__error("Failed to mmap the events: %s\n", str_error_r(errno, sbuf, sizeof(sbuf)));
    perf_evlist__close(evlist);
    goto out;
  }
  rc = 0;
out:
  return rc;
}