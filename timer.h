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
  struct timeval now;

  if (gettimeofday(&now, NULL) == -1) {
    php_error(E_ERROR, "gettimeofday error.");
    return 0ULL;
  }

  return (uint64) now.tv_sec * 1000ULL + (uint64) now.tv_usec / 1000ULL;
}

static zend_always_inline uint64 current_time_milliseconds() {
  struct timeval now;

  if (gettimeofday(&now, NULL) == -1) {
    php_error(E_ERROR, "gettimeofday error.");
    return 0ULL;
  }

  return (uint64) (now.tv_sec * 1000000ULL + now.tv_usec);
}

static zend_always_inline uint64 current_time_nano_seconds() {
  struct timeval now;

  if (gettimeofday(&now, NULL) == -1) {
    php_error(E_ERROR, "gettimeofday error.");
    return 0ULL;
  }

  return (uint64) (now.tv_sec * 1000000000ULL + now.tv_usec);
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

  tsc_start = current_time_milliseconds();
  do {
    for (i = 0; i < 1000000; i++);
    if (gettimeofday(&end, 0)) {
      php_error(E_ERROR, "gettimeofday error.");
      return 0.0;
    }
    tsc_end = current_time_milliseconds();
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
