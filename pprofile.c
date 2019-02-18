/* pprofile extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_pprofile.h"

ZEND_DECLARE_MODULE_GLOBALS(pprofile)

#include "tracing.h"

static void (*_zend_execute_ex)(zend_execute_data *execute_data);
static void (*_zend_execute_internal)(zend_execute_data *execute_data, zval *return_val);
ZEND_DLEXPORT void pprofile_execute_internal(zend_execute_data *execute_data, zval *return_value);
ZEND_DLEXPORT void pprofile_execute_ex(zend_execute_data *execute_data);

PHP_FUNCTION (pprofile_enable) {
  zend_long flags = 0;

  if (zend_parse_parameters(ZEND_NUM_ARGS () TSRMLS_CC, "|l", &flags) == FAILURE) {
    return;
  }

  tracing_begin(flags TSRMLS_CC);
  tracing_enter_root_frame(TSRMLS_C);
}

PHP_FUNCTION (pprofile_disable) {
  tracing_end(TSRMLS_C);

  array_init(return_value);

  tracing_call_graph_append_to_array(return_value TSRMLS_CC);
}

PHP_GINIT_FUNCTION (pprofile) {
  pprofile_globals->root = NULL;
  pprofile_globals->call_graph_frames = NULL;
  pprofile_globals->frame_free_list = NULL;
}

PHP_MSHUTDOWN_FUNCTION (pprofile) {
  return SUCCESS;
}

PHP_RINIT_FUNCTION (pprofile) {
#if defined(ZTS) && defined(COMPILE_DL_PPROFILE)
  ZEND_TSRMLS_CACHE_UPDATE();
#endif

  tracing_request_init(TSRMLS_C);
  tracing_determine_clock_source(TSRMLS_C);

  return SUCCESS;
}

PHP_MINIT_FUNCTION (pprofile) {
  REGISTER_LONG_CONSTANT("PPROFILE_FLAGS_MEMORY",
                         PPROFILE_FLAGS_MEMORY,
                         CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("PPROFILE_FLAGS_MEMORY_MU",
                         PPROFILE_FLAGS_MEMORY_MU,
                         CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("PPROFILE_FLAGS_MEMORY_PMU",
                         PPROFILE_FLAGS_MEMORY_PMU,
                         CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("PPROFILE_FLAGS_CPU",
                         PPROFILE_FLAGS_CPU,
                         CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("PPROFILE_FLAGS_NO_BUILTINS",
                         PPROFILE_FLAGS_NO_BUILTINS,
                         CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("PPROFILE_FLAGS_MEMORY_ALLOC",
                         PPROFILE_FLAGS_MEMORY_ALLOC,
                         CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("PPROFILE_FLAGS_MEMORY_ALLOC_AS_MU",
                         PPROFILE_FLAGS_MEMORY_ALLOC_AS_MU,
                         CONST_CS | CONST_PERSISTENT);

  _zend_execute_internal = zend_execute_internal;
  zend_execute_internal = pprofile_execute_internal;

  _zend_execute_ex = zend_execute_ex;
  zend_execute_ex = pprofile_execute_ex;

  return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION (pprofile) {
  int i = 0;
  pprofile_call_graph_bucket *bucket;

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

  return SUCCESS;
}

PHP_MINFO_FUNCTION (pprofile) {
  php_info_print_table_start();
  php_info_print_table_header(2, "pprofile support", "enabled");
  php_info_print_table_end();
}

ZEND_DLEXPORT void pprofile_execute_internal(zend_execute_data *execute_data, zval *return_value) {
  int is_profiling = 1;

  if (PPRG(enabled) || (PPRG(flags) & PPROFILE_FLAGS_NO_BUILTINS) > 0) {
    execute_internal(execute_data, return_value TSRMLS_CC);
    return;
  }

  is_profiling = tracing_enter_frame_call_graph(NULL, execute_data TSRMLS_CC);

  if (!_zend_execute_internal) {
    execute_internal(execute_data, return_value TSRMLS_CC);
  } else {
    _zend_execute_internal(execute_data, return_value TSRMLS_CC);
  }

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
    PHP_FE(pprofile_enable, NULL)
    PHP_FE(pprofile_disable, NULL)
    PHP_FE_END
};

zend_module_entry pprofile_module_entry = {
    STANDARD_MODULE_HEADER,
    "pprofile",
    pprofile_functions,
    NULL,
    NULL,
    PHP_RINIT(pprofile),
    NULL,
    PHP_MINFO(pprofile),
    PHP_PPROFILE_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_PPROFILE
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(pprofile)
#endif

