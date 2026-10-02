/*
 * crashlog_posix.hpp
 * CraftOS-Tweaked
 *
 * This file defines the helper that the POSIX crash handlers use to save a
 * crash log. Nothing is ever uploaded.
 *
 * This code is licensed under the GNU AGPL v3.0 or later (AGPL-3.0-or-later).
 * Copyright (c) 2026 slammingprogramming.
 */

#ifndef CRASHLOG_POSIX_HPP
#define CRASHLOG_POSIX_HPP

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "util.hpp"

// Saves a crash log for signal `sig`. `extra` is added to the details (lines ending in \n, or ""); `frames` is the
// backtrace to record. Does nothing if the crash log folder is not available. This runs inside a signal handler,
// so it sticks to open/write/backtrace_symbols_fd, like the code it replaces.
static inline void saveCrashLogPosix(const char * platform, int sig, const char * extra, void * const * frames, int count) {
    const int fd = openCrashLog();
    if (fd < 0) return;
    char reason[96];
    snprintf(reason, sizeof(reason), "%s (%d)", strsignal(sig), sig);
    char report[2048];
    size_t n = beginCrashReport(report, sizeof(report), platform, reason);
    n = crashReportf(report, sizeof(report), n, "Details:\n%sLast C function: %s\n", extra, lastCFunction);
    if (!loadingPlugin.empty()) n = crashReportf(report, sizeof(report), n, "Plugin being loaded: %s\n", loadingPlugin.c_str());
    n = crashReportf(report, sizeof(report), n, "Backtrace:\n");
    if (write(fd, report, n) < 0) {}
    backtrace_symbols_fd(frames, count, fd);
    close(fd);
}

#endif
