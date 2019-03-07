//
// Created by Tairy on 2019-02-27.
//

#include "php.h"
#include "php_pprofile.h"
#include "common.h"

extern ZEND_DECLARE_MODULE_GLOBALS(pprofile);

#include "buffer.h"
#include "stream_wrapper.h"

static int real_php_log_buffer(zval *msg_buffer, char *opt, size_t opt_len TSRMLS_DC) {
  php_stream * stream = NULL;
  HashTable *ht;

  zend_ulong num_key;
  zend_string * str_key;
  zval * entry;

  stream = process_stream(opt, opt_len TSRMLS_CC);

  if (stream == NULL) {
    return FAILURE;
  }

  ht = HASH_OF(msg_buffer);
  ZEND_HASH_FOREACH_KEY_VAL(ht, num_key, str_key, entry)
      {
        zend_string * s = zval_get_string(entry);
        php_stream_write(stream, ZSTR_VAL(s), ZSTR_LEN(s));
        zend_string_release(s);
      }
  ZEND_HASH_FOREACH_END();

  return SUCCESS;
}

void pprofile_init_buffer_switch(TSRMLS_D) {
  PPRG(enable_buffer_real) = FAILURE;

  if (SUCCESS == check_sapi_is_cli(TSRMLS_C) && PPRG(buffer_disabled_in_cli)) {
    return;
  }

  if (PPRG(use_buffer) && PPRG(buffer_size) > 0) {
    PPRG(enable_buffer_real) = SUCCESS;
    return;
  }

  return;
}

int pprofile_check_buffer_enable(TSRMLS_D) {
  return SUCCESS == PPRG(enable_buffer_real);
}

int pprofile_buffer_set(char *log_info, size_t log_info_len, char *path, size_t path_len) {
  zval
  new_array;
  zval * _buffer_data_;

  if (IS_ARRAY != Z_TYPE(PPRG(buffer))) {
    return 0;
  }

  if (zend_hash_num_elements(Z_ARRVAL(PPRG(buffer))) < 1) {
    array_init(&new_array);
    PPROFILE_ADD_NEXT_INDEX_STRINGL(&new_array, log_info, log_info_len);
    PPROFILE_ADD_ASSOC_ZVAL_EX(PPRG(buffer), path, path_len, new_array);
  } else {
    if ((_buffer_data_ = zend_hash_str_find_ptr(Z_ARRVAL(PPRG(buffer)), path, path_len)) != NULL) {
      PPROFILE_ADD_NEXT_INDEX_STRINGL((void *) &_buffer_data_, log_info, log_info_len);
    } else {
      array_init(&new_array);
      PPROFILE_ADD_NEXT_INDEX_STRINGL(&new_array, log_info, log_info_len);
      PPROFILE_ADD_ASSOC_ZVAL_EX(PPRG(buffer), path, path_len, new_array);
    }
  }

  if (PPRG(buffer_size) > 0) {
    PPRG(buffer_count)++;

    if (PPRG(buffer_count) >= PPRG(buffer_size)) {
      pprofile_shutdown_buffer(PPROFILE_BUFFER_RE_INIT_YES TSRMLS_CC);
    }
  }

  return SUCCESS;
}

void pprofile_init_buffer(TSRMLS_D) {
  zval * z_buffer;

  if (pprofile_check_buffer_enable(TSRMLS_C)) {
    PPRG(buffer_count) = 0;
    array_init(&PPRG(buffer));
  }
}

void pprofile_shutdown_buffer(int re_init TSRMLS_DC) {
  HashTable *ht;

  zend_ulong num_key;
  zend_string * str_key;
  zval * entry;

  if (pprofile_check_buffer_enable(TSRMLS_C)) {
    if (PPRG(buffer_count) < 1) {
      return;
    }

    ht = Z_ARRVAL(PPRG(buffer));
    ZEND_HASH_FOREACH_KEY_VAL(ht, num_key, str_key, entry)
        {
          real_php_log_buffer(entry, ZSTR_VAL(str_key), ZSTR_LEN(str_key) TSRMLS_CC);
        }
    ZEND_HASH_FOREACH_END();
  }

  if (re_init == PPROFILE_BUFFER_RE_INIT_YES) {
    pprofile_clear_buffer(TSRMLS_C);
    pprofile_init_buffer(TSRMLS_C);
  }
}

void pprofile_clear_buffer(TSRMLS_D) {
  if (pprofile_check_buffer_enable(TSRMLS_C)) {
    PPRG(buffer_count) = 0;
    PPROFILE_ARRAY_DESTROY(PPRG(buffer));
  }
}