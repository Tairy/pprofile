/* pprofile extension for PHP (c) 2019 tairy */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_pprofile.h"

ZEND_DECLARE_MODULE_GLOBALS(pprofile)

#include "appender.h"
#include "buffer.h"
#include "common.h"
#include "logger.h"
#include "stream_wrapper.h"
#include "tracing.h"

PHP_INI_BEGIN()
        STD_PHP_INI_BOOLEAN
        ("pprofile.use_buffer", "0", PHP_INI_SYSTEM, OnUpdateBool, use_buffer, zend_pprofile_globals, pprofile_globals)
        STD_PHP_INI_ENTRY("pprofile.buffer_size",
                          "0",
                          PHP_INI_ALL,
                          OnUpdateLongGEZero,
                          buffer_size,
                          zend_pprofile_globals,
                          pprofile_globals)
        STD_PHP_INI_BOOLEAN("pprofile.buffer_disabled_in_cli",
                            "0",
                            PHP_INI_SYSTEM,
                            OnUpdateBool,
                            buffer_disabled_in_cli,
                            zend_pprofile_globals,
                            pprofile_globals)

        STD_PHP_INI_ENTRY("pprofile.appender",
                          "1",
                          PHP_INI_SYSTEM,
                          OnUpdateLongGEZero,
                          appender,
                          zend_pprofile_globals,
                          pprofile_globals)
        STD_PHP_INI_ENTRY("pprofile.appender_retry",
                          "0",
                          PHP_INI_ALL,
                          OnUpdateLongGEZero,
                          appender_retry,
                          zend_pprofile_globals,
                          pprofile_globals)
        STD_PHP_INI_ENTRY("pprofile.remote_host",
                          "127.0.0.1",
                          PHP_INI_ALL,
                          OnUpdateString,
                          remote_host,
                          zend_pprofile_globals,
                          pprofile_globals)
        STD_PHP_INI_ENTRY("pprofile.remote_port",
                          "514",
                          PHP_INI_ALL,
                          OnUpdateLongGEZero,
                          remote_port,
                          zend_pprofile_globals,
                          pprofile_globals)
        STD_PHP_INI_ENTRY("pprofile.remote_timeout",
                          "1",
                          PHP_INI_SYSTEM,
                          OnUpdateLongGEZero,
                          remote_timeout,
                          zend_pprofile_globals,
                          pprofile_globals)

PHP_INI_END()

static void (*_zend_execute_ex)(zend_execute_data *execute_data);
static void (*_zend_execute_internal)(zend_execute_data *execute_data, zval *return_val);
ZEND_DLEXPORT void pprofile_execute_internal(zend_execute_data *execute_data, zval *return_value);
ZEND_DLEXPORT void pprofile_execute_ex(zend_execute_data *execute_data);

PHP_FUNCTION (pprofile_start) {
  zend_long flags = 0;

  if (zend_parse_parameters(ZEND_NUM_ARGS () TSRMLS_CC, "|l", &flags) == FAILURE) {
    return;
  }

  tracing_begin(flags TSRMLS_CC);
  tracing_enter_root_frame(TSRMLS_C);
}

PHP_FUNCTION (pprofile_end) {
  tracing_end(TSRMLS_C);

  array_init(return_value);

  tracing_call_graph_append_to_array(return_value TSRMLS_CC);

  pprofile_log_ex(return_value);
}

PHP_GINIT_FUNCTION (pprofile) {
  memset(pprofile_globals, 0, sizeof(zend_pprofile_globals));
  pprofile_globals->root = NULL;
  pprofile_globals->call_graph_frames = NULL;
  pprofile_globals->frame_free_list = NULL;
}

PHP_MSHUTDOWN_FUNCTION (pprofile) {

  UNREGISTER_INI_ENTRIES();
  return SUCCESS;
}

PHP_RINIT_FUNCTION (pprofile) {
#if defined(ZTS) && defined(COMPILE_DL_PPROFILE)
  ZEND_TSRMLS_CACHE_UPDATE();
#endif

  tracing_request_init(TSRMLS_C);
  tracing_determine_clock_source(TSRMLS_C);

  pprofile_init_logger(TSRMLS_C);
  pprofile_init_logger_list(TSRMLS_C);

  pprofile_init_stream_list(TSRMLS_C);

  return SUCCESS;
}

PHP_MINIT_FUNCTION (pprofile) {
  _zend_execute_internal = zend_execute_internal;
  zend_execute_internal = pprofile_execute_internal;

  _zend_execute_ex = zend_execute_ex;
  zend_execute_ex = pprofile_execute_ex;

  pprofile_init_buffer_switch(TSRMLS_C);

  REGISTER_INI_ENTRIES();

  return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION (pprofile) {
  int i = 0;
  pprofile_call_graph_bucket_t *bucket;

  tracing_end(TSRMLS_C);

  for (i = 0; i < PPROFILE_CALL_GRAPH_SLOTS; i++) {
    bucket = PPRG(call_graph_buckets)[i];

    while (bucket) {
      PPRG(call_graph_buckets)[i] = bucket->next;
      tracing_call_graph_bucket_free(bucket);
      bucket = PPRG(call_graph_buckets)[i];
    }
  }

  tracing_request_shutdown();

  pprofile_free_logger(TSRMLS_C);
  pprofile_free_logger_list(TSRMLS_C);

  pprofile_free_stream(PPROFILE_STREAM_LIST_DESTROY_YES, PPROFILE_CLOSE_LOGGER_STREAM_MOD_ALL, NULL TSRMLS_CC);

  return SUCCESS;
}

PHP_MINFO_FUNCTION (pprofile) {
  php_info_print_table_start();
  php_info_print_table_header(2, "pprofile support", "enabled");
  php_info_print_table_end();

  DISPLAY_INI_ENTRIES();
}

ZEND_DLEXPORT void pprofile_execute_internal(zend_execute_data *execute_data, zval *return_value) {
  int is_profiling = 1;

  if (!PPRG(enabled) || (PPRG(flags) & PPROFILE_FLAGS_NO_BUILTINS) > 0) {
    execute_internal(execute_data, return_value TSRMLS_CC);
    return;
  }

  // 开始执行一个函数，记录起始值
  is_profiling = tracing_enter_frame_call_graph(NULL, execute_data TSRMLS_CC);

  // 执行函数
  if (!_zend_execute_internal) {
    execute_internal(execute_data, return_value TSRMLS_CC);
  } else {
    _zend_execute_internal(execute_data, return_value TSRMLS_CC);
  }

  // 结束执行，记录结束值
  if (is_profiling == 1 && PPRG(call_graph_frames)) {
    tracing_exit_frame_call_graph(TSRMLS_C);
  }
}

ZEND_DLEXPORT void pprofile_execute_ex(zend_execute_data *execute_data) {
  zend_execute_data *real_execute_data = execute_data;
  int is_profiling = 0;

  if (!PPRG(enabled)) {
    _zend_execute_ex(execute_data TSRMLS_CC);
    return;
  }

  is_profiling = tracing_enter_frame_call_graph(NULL, real_execute_data TSRMLS_CC);

  _zend_execute_ex(execute_data TSRMLS_CC);

  if (is_profiling == 1 && PPRG(call_graph_frames)) {
    tracing_exit_frame_call_graph(TSRMLS_C);
  }
}

static const zend_function_entry pprofile_functions[] = {
    PHP_FE(pprofile_start, NULL)
    PHP_FE(pprofile_end, NULL)
    PHP_FE_END
};

zend_module_entry pprofile_module_entry = {
    STANDARD_MODULE_HEADER,
    "pprofile",
    pprofile_functions,
    PHP_MINIT(pprofile),
    PHP_MSHUTDOWN(pprofile),
    PHP_RINIT(pprofile),
    PHP_RSHUTDOWN(pprofile),
    PHP_MINFO(pprofile),
    PHP_PPROFILE_VERSION,
    PHP_MODULE_GLOBALS(pprofile),
    PHP_GINIT(pprofile),
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_PPROFILE
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(pprofile)
#endif

