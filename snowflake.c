//
// Created by Tairy on 2019-03-13.
//

#include "php.h"
#include "php_pprofile.h"
#include "shm.h"
#include "snowflake.h"
#include "spinlock.h"

extern ZEND_DECLARE_MODULE_GLOBALS(pprofile);

int pprofile_init_snowflake() {
  PPRG(shmem).size = sizeof(struct pprofile_snowflake_context_t);
  if (shm_alloc(&PPRG(shmem)) == -1) {
    return -1;
  }

  PPRG(context) = (struct pprofile_snowflake_context_t *) PPRG(shmem).addr;
  PPRG(context)->lock = 0;

  PPRG(context)->sequence = 0;
  PPRG(context)->last_ts = 0ULL;
//  PPRG(context)->data_center_id =

  PPRG(context)->worker_id_bits = 5;
  PPRG(context)->data_center_id_bits = 5;
  PPRG(context)->sequence_bits = 12;

  PPRG(context)->worker_id_shift = PPRG(context)->sequence_bits;
  PPRG(context)->data_center_id_shift = PPRG(context)->sequence_bits
      + PPRG(context)->worker_id_bits;
  PPRG(context)->timestamp_left_shift = PPRG(context)->sequence_bits
      + PPRG(context)->worker_id_bits
      + PPRG(context)->data_center_id_bits;

  PPRG(context)->sequence_mask = -1 ^ (-1 << PPRG(context)->data_center_id_bits);

  spin_init();

  return 0;
}

void pprofile_free_snowflake() {
  shm_free(&PPRG(shmem));
}
