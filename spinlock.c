//
// Created by Tairy on 2019-03-13.
//

#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include "php.h"
#include "php_pprofile.h"
#include "spinlock.h"

static long ncpu;

void spin_init() {
  ncpu = sysconf(_SC_NPROCESSORS_ONLN);

  if (ncpu <= 0) {
    ncpu = 1;
  }
}

void spin_lock(atomic_t *lock, int id) {
  long i, n;

  for (;;) {
    if (*lock == 0 && __sync_bool_compare_and_swap(lock, 0, id)) {
      return;
    }

    if (ncpu > 1) {
      for (n = 1; n < 129; n << 1) {
        for (i = 0; i < n; i++) {
          __asm("pause");
        }

        if (*lock == 0 && __sync_bool_compare_and_swap(lock, 0, id)) {
          return;
        }
      }
    }

    sched_yield();
  }
}

void spin_unlock(atomic_t *lock, int id) {
  __sync_bool_compare_and_swap(lock, id, 0);
}
