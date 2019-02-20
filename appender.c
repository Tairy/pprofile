//
// Created by Tairy on 2019-02-20.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_pprofile.h"

extern ZEND_DECLARE_MODULE_GLOBALS(pprofile);

#include "appender.h"

static zend_always_inline int appender_handle_udp_tcp(pprofile_logger_entry_t *logger,
                                                      zend_class_entry *ce TSRMLS_DC) {
}

zend_always_inline pprofile_log_ex(int argc, char *message, int message_len, zend_class_entry *ce TSRMLS_DC) {
  pprofile_logger_entry_t *logger;

  logger = PPRG(last_logger);

  return appender_handle_udp_tcp(logger, ce);
}

