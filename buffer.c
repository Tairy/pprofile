//
// Created by Tairy on 2019-02-27.
//

#include "php.h"
#include "php_pprofile.h"
#include "common.h"

extern ZEND_DECLARE_MODULE_GLOBALS(pprofile);

#include "buffer.h"

void pprofile_init_buffer_switch(TSRMLS_D) {
  PPRG(enable_buffer_real) = FAILURE;

  if (SUCCESS == check_sapi_is_cli(TSRMLS_C) && PPRG(buffer_disabled_in_cli)) {
    return;
  }

  if (PPRG(use_buffer) && PPRG(buffer_size) > 0) {
    PPRG(enable_buffer_real) = SUCCESS;
    return;
  }

  return;
}

int pprofile_check_buffer_enable(TSRMLS_D) {
  return SUCCESS == PPRG(enable_buffer_real);
}
