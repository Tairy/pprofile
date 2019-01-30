/* pprofile extension for PHP (c) 2019 tairy */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_pprofile.h"

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
    ZEND_PARSE_PARAMETERS_START(0, 0) \
    ZEND_PARSE_PARAMETERS_END()
#endif

typedef struct pp_global_t {

};

PHP_MINFO_FUNCTION(pprofile)
    {

    }

/* {{{ void pprofile_test1()
 */
PHP_FUNCTION(pprofile_test1)
    {
        ZEND_PARSE_PARAMETERS_NONE();

        php_printf("The extension %s is loaded and working!\r\n", "pprofile");
    }
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(pprofile)
    {
#if defined(ZTS) && defined(COMPILE_DL_PPROFILE)
        ZEND_TSRMLS_CACHE_UPDATE();
#endif

        return SUCCESS;
    }
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(pprofile)
    {
        php_info_print_table_start ();
        php_info_print_table_header(2, "pprofile support", "enabled");
        php_info_print_table_end();
    }
/* }}} */

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO(arginfo_pprofile_test1,

0)

ZEND_END_ARG_INFO ()

ZEND_BEGIN_ARG_INFO(arginfo_pprofile_test2,

0)
ZEND_ARG_INFO(0, str)

ZEND_END_ARG_INFO ()
/* }}} */

/* {{{ pprofile_functions[]
 */
static const zend_function_entry pprofile_functions[] = {
    PHP_FE (pprofile_test1, arginfo_pprofile_test1)
    PHP_FE(pprofile_test2, arginfo_pprofile_test2)
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
    PHP_RINIT (pprofile),            /* PHP_RINIT - Request initialization */
    NULL,                            /* PHP_RSHUTDOWN - Request shutdown */
    PHP_MINFO (pprofile),            /* PHP_MINFO - Module info */
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

