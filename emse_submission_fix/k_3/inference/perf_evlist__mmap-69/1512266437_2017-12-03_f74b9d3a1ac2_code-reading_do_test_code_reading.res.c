static int do_test_code_reading(bool try_kcore)
{
	struct machine *machine;
	struct thread *thread;
	struct record_opts opts = {
		.mmap_pages	     = UINT_MAX,
		.user_freq	     = UINT_MAX,
		.user_interval	     = ULLONG_MAX,
		.freq		     = 500,
		.target		     = {
			.uses_mmap   = true,
		},
	};
	struct state state = {
		.done_cnt = 0,
	};
	struct thread_map *threads = NULL;
	struct cpu_map *cpus = NULL;
	struct perf_evlist *evlist = NULL;
	struct perf_evsel *evsel = NULL;
	int err = -1, ret;
	pid_t pid;
	struct map *map;
	bool have_vmlinux, have_kcore, excl_kernel = false;
	pid = getpid();
	machine = machine__new_host();
	ret = machine__create_kernel_maps(machine);
	if (ret < 0) {
		pr_debug("machine__create_kernel_maps failed\n");
		goto out_err;
	}
	/* Force the use of kallsyms instead of vmlinux to try kcore */
	if (try_kcore)
		symbol_conf.kallsyms_name = "/proc/kallsyms";
	/* Load kernel map */
	map = machine__kernel_map(machine);
	ret = map__load(map);
	if (ret < 0) {
		pr_debug("map__load failed\n");
		goto out_err;
	}
	have_vmlinux = dso__is_vmlinux(map->dso);
	have_kcore = dso__is_kcore(map->dso);
	/* 2nd time through we just try kcore */
	if (try_kcore && !have_kcore)
		return TEST_CODE_READING_NO_KCORE;
	/* No point getting kernel events if there is no kernel object */
	if (!have_vmlinux && !have_kcore)
		excl_kernel = true;
	threads = thread_map__new_by_tid(pid);
	if (!threads) {
		pr_debug("thread_map__new_by_tid failed\n");
		goto out_err;
	}
	ret = perf_event__synthesize_thread_map(NULL, threads,
						perf_event__process, machine, false, 500);
	if (ret < 0) {
		pr_debug("perf_event__synthesize_thread_map failed\n");
		goto out_err;
	}
	thread = machine__findnew_thread(machine, pid, pid);
	if (!thread) {
		pr_debug("machine__findnew_thread failed\n");
		goto out_put;
	}
	cpus = cpu_map__new(NULL);
	if (!cpus) {
		pr_debug("cpu_map__new failed\n");
		goto out_put;
	}
	while (1) {
		const char *str;
		evlist = perf_evlist__new();
		if (!evlist) {
			pr_debug("perf_evlist__new failed\n");
			goto out_put;
		}
		perf_evlist__set_maps(evlist, cpus, threads);
		if (excl_kernel)
			str = "cycles:u";
		else
			str = "cycles";
		pr_debug("Parsing event '%s'\n", str);
		ret = parse_events(evlist, str, NULL);
		if (ret < 0) {
			pr_debug("parse_events failed\n");
			goto out_put;
		}
		perf_evlist__config(evlist, &opts, NULL);
		evsel = perf_evlist__first(evlist);
		evsel->attr.comm = 1;
		evsel->attr.disabled = 1;
		evsel->attr.enable_on_exec = 0;
		ret = perf_evlist__open(evlist);
		if (ret < 0) {
			if (!excl_kernel) {
				excl_kernel = true;
				/*
				 * Both cpus and threads are now owned by evlist
				 * and will be freed by following perf_evlist__set_maps
				 * call. Getting refference to keep them alive.
				 */
				cpu_map__get(cpus);
				thread_map__get(threads);
				perf_evlist__set_maps(evlist, NULL, NULL);
				perf_evlist__delete(evlist);
				evlist = NULL;
				continue;
			}
			if (verbose > 0) {
				char errbuf[512];
				perf_evlist__strerror_open(evlist, errno, errbuf, sizeof(errbuf));
				pr_debug("perf_evlist__open() failed!\n%s\n", errbuf);
			}
			goto out_put;
		}
		break;
	}
	ret = perf_evlist__mmap(evlist, UINT_MAX);
	if (ret < 0) {
		pr_debug("perf_evlist__mmap failed\n");
		goto out_put;
	}
	perf_evlist__enable(evlist);
	do_something();
	perf_evlist__disable(evlist);
	ret = process_events(machine, evlist, &state);
	if (ret < 0)
		goto out_put;
	if (!have_vmlinux && !have_kcore && !try_kcore)
		err = TEST_CODE_READING_NO_KERNEL_OBJ;
	else if (!have_vmlinux && !try_kcore)
		err = TEST_CODE_READING_NO_VMLINUX;
	else if (excl_kernel)
		err = TEST_CODE_READING_NO_ACCESS;
	else
		err = TEST_CODE_READING_OK;
out_put:
	thread__put(thread);
out_err:
	if (evlist) {
		perf_evlist__delete(evlist);
	} else {
		cpu_map__put(cpus);
		thread_map__put(threads);
	}
	machine__delete_threads(machine);
	machine__delete(machine);
	return err;
}