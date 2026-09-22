// must come before any system header so sched.h exposes sched_getaffinity/CPU_COUNT
#if defined(__linux__)
#define _GNU_SOURCE
#endif

#include "cpucount.h"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#elif defined(__linux__)
#include <sched.h>
#include <unistd.h>
#else
#include <unistd.h>
#endif

int cpuCount(void) {
#if defined(_WIN32)
    // counts across all processor groups, so machines with >64 CPUs aren't capped at 64
    DWORD n = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
    return n > 0 ? (int)n : 1;
#elif defined(__APPLE__)
    int n;
    size_t len = sizeof n;
    if (sysctlbyname("hw.logicalcpu", &n, &len, NULL, 0) == 0 && n > 0) return n;
    return 1;
#elif defined(__linux__)
    // respects taskset / container cpusets, unlike sysconf
    cpu_set_t set;
    if (sched_getaffinity(0, sizeof set, &set) == 0) {
        int n = CPU_COUNT(&set);
        if (n > 0) return n;
    }
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return n > 0 ? (int)n : 1;
#else
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return n > 0 ? (int)n : 1;
#endif
}

// cpu count helper written by claude.