#include <stdio.h>
#include <ctype.h>
#include <sys/utsname.h>

#include "bench.h"

static int isfname(char c)
{
	return isalnum(c) || c == '-';
}

static void cvtfname(char *str)
{
	int r = 0, w = 0;
	while (str[r] != '\0') {
		char c = str[r++];
		if (!isfname(c)) {
			c = '_';
			if (r > 1 && !isfname(str[r-2])) {
				continue;
			}
		}
		str[w++] = c;
	}
	str[w] = '\0';
}

int main(int argc, char const *argv[])
{
	char const *os_name;
	char const *os_version;
	char const *timer_name;
	char const *cpu_info;

	// ---- OS Name & Version ----
	struct utsname mi;
	if (uname(&mi) == 0) {
		cvtfname(mi.sysname);
		cvtfname(mi.release);
		os_name = mi.sysname;
		os_version = mi.release;
	} else {
		os_name = "unknown";
		os_version = "unknown";
	}
	
	// ---- Timing method ----
#if BENCH_TIMER == BENCH_GETTIMEOFDAY
	timer_name = "gettimeofday";
#elif BENCH_TIMER == BENCH_POSIX_MONOTONIC
	timer_name = "posix_monotonic";
#else
	timer_name = "unknown";
#endif
	
	// ---- CPU brand ----
	cpu_info = "unknown";
	char cpu_buf[128];
	FILE *proc_cpuinfo = fopen("/proc/cpuinfo", "r");
	if (proc_cpuinfo != NULL) {
		for (;;) {
			char *line = NULL; // No need to free later, OS does that on exit anyway.
			size_t linesz = 0;
			ssize_t rc = getline(&line, &linesz, proc_cpuinfo);
			if (rc == -1) break;
			int ec = sscanf(line, "model name : %[^\n]\n", cpu_buf);
			if (ec > 0) {
				cvtfname(cpu_buf);
				cpu_info = cpu_buf;
				break;
			}
		}
		fclose(proc_cpuinfo);
	}
	
	// ---- Putting it all together ----
	printf("%s_%s_%s_%s", os_name, os_version, timer_name, cpu_info);
 	return 0;
}

