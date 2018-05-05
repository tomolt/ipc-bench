/*
    Generate a filename for the measurement results
	based on the machines specification.


    Copyright (c) 2018 Thomas Oltmann <thomas.oltmann.hhg@gmail.com>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
*/

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
#if BMB_TIMER == BMB_GETTIMEOFDAY
	timer_name = "gettimeofday";
#elif BMB_TIMER == BMB_POSIX_MONOTONIC
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

