//
// Created by Tairy on 2019-03-13.
//

#include "php.h"
#include "php_pprofile.h"
extern ZEND_DECLARE_MODULE_GLOBALS(pprofile);

#include "shm.h"
#include "snowflake.h"
#include "spinlock.h"
#include "timer.h"

int pprofile_init_snowflake() {
  PPRG(shmem).size = sizeof(struct pprofile_snowflake_context_t);

  if (shm_alloc(&PPRG(shmem)) == -1) {
    return -1;
  }

  pprofile_globals.snowflake_context = (struct pprofile_snowflake_context_t *) pprofile_globals.shmem.addr;

  pprofile_globals.snowflake_context->lock = 0;
  pprofile_globals.snowflake_context->sequence = 0;
  pprofile_globals.snowflake_context->last_ts = 0ULL;

  pprofile_globals.snowflake_context->worker_id_bits = 5;
  pprofile_globals.snowflake_context->data_center_id_bits = 5;
  pprofile_globals.snowflake_context->sequence_bits = 12;

  pprofile_globals.snowflake_context->worker_id_shift = pprofile_globals.snowflake_context->sequence_bits;
  pprofile_globals.snowflake_context->data_center_id_shift = pprofile_globals.snowflake_context->sequence_bits
      + pprofile_globals.snowflake_context->worker_id_bits;
  pprofile_globals.snowflake_context->timestamp_left_shift = pprofile_globals.snowflake_context->sequence_bits
      + pprofile_globals.snowflake_context->worker_id_bits
      + pprofile_globals.snowflake_context->data_center_id_bits;

  pprofile_globals.snowflake_context->sequence_mask = -1 ^ (-1 << pprofile_globals.snowflake_context->data_center_id_bits);

  spin_init();

  return 0;
}

zend_always_inline static uint64 skip_next_millis(uint64 last_ts) {
  uint64 current;

  for (;;) {
    current = current_timestamp();
    if (current > last_ts) {
      break;
    }
  }

  return current;
}

uint64 get_uuid() {
  uint64 current = current_timestamp();
  uint64 retval;

  if (current == 0ULL) {
    return 0ULL;
  }

  spin_lock(&PPRG(snowflake_context)->lock, (int) PPRG(current_pid));

  if (PPRG(snowflake_context)->last_ts == current) {
    PPRG(snowflake_context)->sequence =
        (PPRG(snowflake_context)->sequence + 1) & PPRG(snowflake_context)->sequence_mask;
    if (PPRG(snowflake_context)->sequence == 0) {
      current = skip_next_millis(PPRG(snowflake_context)->last_ts);
    }
  } else {
    PPRG(snowflake_context)->sequence = 0;
  }

  PPRG(snowflake_context)->last_ts = current;
  retval = ((current - PPRG(snowflake_context)->twepoch) << PPRG(snowflake_context)->timestamp_left_shift)
      | (PPRG(snowflake_context)->data_center_id << PPRG(snowflake_context)->data_center_id_shift)
      | (PPRG(snowflake_context)->worker_id << PPRG(snowflake_context)->worker_id_shift)
      | PPRG(snowflake_context)->sequence;

  spin_unlock(&PPRG(snowflake_context)->lock, (int) PPRG(current_pid));

  return retval;
}

void pprofile_free_snowflake() {
  shm_free(&PPRG(shmem));
}
