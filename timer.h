//
// Created by Tairy on 2019-01-31.
//

#ifndef PPROFILE_TIMER_H
#define PPROFILE_TIMER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/time.h>
#include <sys/resource.h>

static zend_always_inline uint64 current_timestamp() {
  struct timeval tv;

  if (gettimeofday(&tv, NULL)) {
    php_error(E_ERROR, "gettimeofday error.");
    return 0;
  }

  return 1000 * (uint64) tv.tv_sec + (uint64) tv.tv_usec / 1000;
}

// TODO：这个函数需要调试
static zend_always_inline uint64 time_milliseconds() {
  struct timespec s;
  uint32 a, d;
  uint64 val;

  struct timeval now;
  if (gettimeofday(&now, NULL) == 0) {
    return (uint64) (now.tv_sec * 1000000 + now.tv_usec);
  }

  return 0;
}

/**
 * Get time delta in microseconds.
 */
static long get_us_interval(struct timeval *start, struct timeval *end) {
  return (((end->tv_sec - start->tv_sec) * 1000000)
      + (end->tv_usec - start->tv_usec));
}

static zend_always_inline double get_timebase_factor() {
  struct timeval start;
  struct timeval end;
  uint64 tsc_start;
  uint64 tsc_end;
  volatile int i;

  if (gettimeofday(&start, 0)) {
    php_error(E_ERROR, "gettimeofday error.");
    return 0.0;
  }

  tsc_start = time_milliseconds();
  do {
    for (i = 0; i < 1000000; i++);
    if (gettimeofday(&end, 0)) {
      php_error(E_ERROR, "gettimeofday error.");
      return 0.0;
    }
    tsc_end = time_milliseconds();
  } while (get_us_interval(&start, &end) < 5000);

  return (tsc_end - tsc_start) * 1.0 / (get_us_interval(&start, &end));
}

static uint64 cpu_timer() {
  struct rusage ru;
#if defined(CLOCK_PROCESS_CPUTIME_ID)
  struct timespec s;
  if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &s) == 0) {
    return (uint64) (s.tv_sec * 1000000 + s.tv_nsec / 1000);
  }
#endif

  if (getrusage(RUSAGE_SELF, &ru) == 0) {
    return (uint64) (ru.ru_utime.tv_sec * 1000000 + ru.ru_utime.tv_usec + ru.ru_stime.tv_sec * 1000000
        + ru.ru_stime.tv_usec);
  }

  return 0;
}
#endif //PPROFILE_TIMER_H
