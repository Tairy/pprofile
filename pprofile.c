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

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION (pprofile) {
#if defined(ZTS) && defined(COMPILE_DL_PPROFILE)
  ZEND_TSRMLS_CACHE_UPDATE();
#endif

  return SUCCESS;
}
/* }}} */

PHP_MINIT_FUNCTION (pprofile) {
  
  _zend_execute_internal = zend_execute_internal;
  zend_execute_internal = pprofile_execute_internal;

  _zend_execute_ex = zend_execute_ex;
  zend_execute_ex = pprofile_execute_ex;

  return SUCCESS;
}

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION (pprofile) {
  php_info_print_table_start();
  php_info_print_table_header(2, "pprofile support", "enabled");
  php_info_print_table_end();
}
/* }}} */


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

/* {{{ pprofile_functions[]
 */
static const zend_function_entry pprofile_functions[] = {
    PHP_FE(pprofile_enable, NULL)
    PHP_FE_END
};
/* }}} */

/* {{{ pprofile_module_entry
 */
zend_module_entry pprofile_module_entry = {
    STANDARD_MODULE_HEADER,
    "pprofile",                    /* Extension name */
    pprofile_functions,            /* zend_function_entry */
    NULL,                            /* PHP_MINIT - Module initialization */
    NULL,                            /* PHP_MSHUTDOWN - Module shutdown */
    PHP_RINIT(pprofile),            /* PHP_RINIT - Request initialization */
    NULL,                            /* PHP_RSHUTDOWN - Request shutdown */
    PHP_MINFO(pprofile),            /* PHP_MINFO - Module info */
    PHP_PPROFILE_VERSION,        /* Version */
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PPROFILE
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(pprofile)
#endif

