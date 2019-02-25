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
#include "stream_wrapper.h"

static int pprofile_real_log_ex(char *message, size_t message_len, char *opt, size_t opt_len TSRMLS_DC) {
  size_t
  written;

  php_stream * stream = NULL;

  stream = process_stream(opt, opt_len TSRMLS_CC);
  if (!stream) {
    return FAILURE;
  }

  written = php_stream_write(stream, message, message_len);
  return SUCCESS;
}

static int pprofile_real_buffer_log_ex(char *message,
                                       size_t message_len,
                                       char *log_file_path,
                                       size_t log_file_path_len) {
  return pprofile_real_log_ex(message, message_len, log_file_path, log_file_path_len TSRMLS_CC);
}

static int appender_handle_file(char *message, size_t message_len, pprofile_logger_entry_t *logger) {

  char *log_file_path, *log_info;

  size_t
  log_file_path_len, log_len;

  log_file_path_len = spprintf(&log_file_path, 0, "%s.log", logger->logger_path);
  log_len = spprintf(&log_info, 0, "%s", message);

  if (pprofile_real_buffer_log_ex(log_info, log_len, log_file_path, log_file_path_len + 1) == FAILURE) {
    efree(log_file_path);
    efree(log_info);
    return FAILURE;
  }

  efree(log_file_path);
  efree(log_info);
  return SUCCESS;
}

void pprofile_log_ex(zval *log_info TSRMLS_DC) {
  smart_str performance_log = {0};

  php_json_encode(&performance_log, log_info, 0);

  PPRG(last_logger)->logger_path = "./xxx";

  appender_handle_file(ZSTR_VAL(performance_log.s), ZSTR_LEN(performance_log.s), PPRG(last_logger));
  smart_str_free(&performance_log);
}

