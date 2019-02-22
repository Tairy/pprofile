//
// Created by Tairy on 2019-02-20.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_pprofile.h"
#include "ext/json/php_json.h"
#include "zend_smart_str.h"

extern ZEND_DECLARE_MODULE_GLOBALS(pprofile);

#include "appender.h"

void pprofile_log_ex(zval *log_info TSRMLS_DC) {
  smart_str performance_log = {0};

  php_json_encode(&performance_log, log_info, 0);

//      seaslog_log_ex(3, SEASLOG_INFO, SEASLOG_INFO_INT, SEASLOG_SMART_STR_C(performance_log), seaslog_smart_str_get_len(performance_log), SEASLOG_PERFORMANCE_LOGGER, strlen(SEASLOG_PERFORMANCE_LOGGER)+1, ce TSRMLS_CC);

  smart_str_free(&performance_log);
}

