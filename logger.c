//
// Created by Tairy on 2019-02-21.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_pprofile.h"

extern ZEND_DECLARE_MODULE_GLOBALS(pprofile);

#include "logger.h"

void pprofile_init_logger(TSRMLS_D) {

  // alloc last_logger
  PPRG(last_logger) = ecalloc(1, sizeof(pprofile_logger_entry_t));
}

void pprofile_init_logger_list(TSRMLS_D) {
  array_init(&PPRG(logger_list));
}

void pprofile_free_logger(TSRMLS_D) {

  // free last_logger
  if (PPRG(last_logger)) {
    if (PPRG(last_logger)->logger) {
      efree(PPRG(last_logger)->logger);
    }

//    if (PPRG(last_logger)->logger_path) {
//      efree(PPRG(last_logger)->logger_path);
//    }

    efree(PPRG(last_logger));
  }
}

void pprofile_free_logger_list(TSRMLS_D) {
  PPROFILE_ARRAY_DESTROY(PPRG(logger_list));
}