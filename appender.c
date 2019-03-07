//
// Created by Tairy on 2019-02-20.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_pprofile.h"
#include "buffer.h"
#include "ext/json/php_json.h"
#include "zend_smart_str.h"

extern ZEND_DECLARE_MODULE_GLOBALS(pprofile);

#include "appender.h"
#include "stream_wrapper.h"

static int pprofile_real_log_ex(char *message, size_t message_len, char *opt, size_t opt_len TSRMLS_DC) {
  size_t
  written;

  int retry = 3;
  php_stream * stream = NULL;

  stream = process_stream(opt, opt_len TSRMLS_CC);
  if (!stream) {
    return FAILURE;
  }

  written = php_stream_write(stream, message, message_len);
  if (written != message_len) {
    if (retry > 0) {
      while (retry > 0) {
        written = php_stream_write(stream, message, message_len);
        if (written == message_len) {
          return SUCCESS;
        } else {
          retry--;
        }
      }
    }

    return FAILURE;
  }
  return SUCCESS;
}

static int pprofile_real_buffer_log_ex(char *message,
                                       size_t message_len,
                                       char *log_file_path,
                                       size_t log_file_path_len) {
  if (pprofile_check_buffer_enable(TSRMLS_C)) {
    pprofile_buffer_set(message, message_len, log_file_path, log_file_path_len TSRMLS_CC);
    return SUCCESS;
  } else {
    return pprofile_real_log_ex(message, message_len, log_file_path, log_file_path_len TSRMLS_CC);
  }
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

static int appender_handle_tcp_udp(char *message, size_t message_len, pprofile_logger_entry_t *logger TSRMLS_DC) {
  char *log_info;
  size_t
  log_len, log_context_len;

  log_len = spprintf(&log_info, 0, "%s", message);

  if (pprofile_real_buffer_log_ex(log_info, log_len, logger->logger, logger->logger_len) == FAILURE) {
    efree(log_info);
    return FAILURE;
  }

  efree(log_info);
  return SUCCESS;
}

void pprofile_log_ex(zval *log_info TSRMLS_DC) {
  smart_str performance_log = {0};

  php_json_encode(&performance_log, log_info, 0);

  switch PPRG(appender) {
    case PPROFILE_APPENDER_TCP:
    case PPROFILE_APPENDER_UDP:
      appender_handle_tcp_udp(ZSTR_VAL(performance_log.s),
                              ZSTR_LEN(performance_log.s),
                              PPRG(last_logger) TSRMLS_CC);
      break;
    case PPROFILE_APPENDER_FILE:
    default:
      
      PPRG(last_logger)->logger_path = "/tmp/xxx";
      appender_handle_file(ZSTR_VAL(performance_log.s),
                           ZSTR_LEN(performance_log.s),
                           PPRG(last_logger)
                           TSRMLS_CC);\
      break;
  }
  smart_str_free(&performance_log);
}

