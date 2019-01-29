/* pprofile extension for PHP (c) 2019 tairy */

#ifndef PHP_PPROFILE_H
# define PHP_PPROFILE_H

extern zend_module_entry pprofile_module_entry;
# define phpext_pprofile_ptr &pprofile_module_entry

# define PHP_PPROFILE_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_PPROFILE)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_PPROFILE_H */

