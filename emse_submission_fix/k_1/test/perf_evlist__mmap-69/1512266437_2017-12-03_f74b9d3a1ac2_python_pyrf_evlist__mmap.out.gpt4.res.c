static PyObject *pyrf_evlist__mmap(struct pyrf_evlist *pevlist, PyObject *args, PyObject *kwargs)
{
  struct perf_evlist *evlist = &pevlist->evlist;
  static char *kwlist[] = {"pages", "overwrite", NULL};
  int pages = 128, overwrite = false;
  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|ii", kwlist, &pages, &overwrite))
    return NULL;
  if (perf_evlist__mmap(evlist, pages) < 0)
    {
      PyErr_SetFromErrno(PyExc_OSError);
      return NULL;
    }
  Py_INCREF(Py_None);
  return Py_None;
}