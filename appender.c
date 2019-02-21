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

zend_always_inline void pprofile_log_ex(zval *log_info) {
  php_printf(log_info);
}

