static int do_test(struct perf_evlist *evlist, int mmap_pages, int *sample_count, int *comm_count)
{
  int err;
  char sbuf[STRERR_BUFSIZE];
  err = perf_evlist__mmap(evlist, mmap_pages);
  if (err < 0)
    {
      pr_debug("perf_evlist__mmap: %s\n", str_error_r(errno, sbuf, sizeof(sbuf)));
      return TEST_FAIL;
    }
  perf_evlist__enable(evlist);
  testcase();
  perf_evlist__disable(evlist);
  err = count_samples(evlist, sample_count, comm_count);
  perf_evlist__munmap(evlist);
  return err;
}