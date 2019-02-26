//
// Created by Tairy on 2019-02-22.
//

#include "php.h"
#include "php_pprofile.h"
#include "php_streams.h"

extern ZEND_DECLARE_MODULE_GLOBALS(pprofile);

#include "stream_wrapper.h"

void pprofile_init_stream_list(TSRMLS_D) {
  zval * z_stream_list;
  array_init(&PPRG(stream_list));
}

int pprofile_free_stream(int destroy, int model, char *opt TSRMLS_DC) {
  php_stream * stream = NULL;
  HashTable *ht;
  pprofile_stream_entry_t *stream_entry;
  int result = FAILURE;
  zend_ulong num_key;

  if (IS_ARRAY == Z_TYPE(PPRG(stream_list))) {
    ht = Z_ARRVAL(PPRG(stream_list));
    ZEND_HASH_FOREACH_NUM_KEY_PTR(ht, num_key, stream_entry)
        {
          if (PPROFILE_CLOSE_LOGGER_STREAM_MOD_ALL == model
              || (PPROFILE_CLOSE_LOGGER_STREAM_MOD_ASSIGN == model && strstr(stream_entry->opt, opt))) {
            stream = stream_entry->stream;
            if (stream) {
              php_stream_close(stream);
              zend_hash_index_del(ht, num_key);
            }
            efree(stream_entry->opt);
            efree(stream_entry);
            result = SUCCESS;
          }
        }
    ZEND_HASH_FOREACH_END();

    if (PPROFILE_STREAM_LIST_DESTROY_YES == destroy) {
      PPROFILE_ARRAY_DESTROY(PPRG(stream_list));
    }
  }

  return result;
}

zend_always_inline php_stream *pprofile_stream_open_wrapper(char *opt TSRMLS_DC) {
  php_stream * stream = NULL;
  int first_create_file = 0;

  if (access(opt, F_OK) != 0) {
    first_create_file = 1;
  }

  stream = php_stream_open_wrapper(opt, "a", IGNORE_URL_WIN, NULL);

  if (stream != NULL) {
    if (first_create_file == 1) {
      VCWD_CHMOD(opt, PPROFILE_FILE_MODE);
    }
  } else {
    // TODO: 异常处理
  }

  return stream;
}

zend_always_inline php_stream *process_stream(char *opt, size_t opt_len TSRMLS_DC) {
  ulong stream_entry_hash;
  php_stream * stream = NULL;
  HashTable *ht_list;
  php_stream_statbuf dest_s;
  pprofile_stream_entry_t *stream_entry;

  stream_entry_hash = zend_inline_hash_func(opt, opt_len);

  ht_list = Z_ARRVAL(PPRG(stream_list));
  if ((stream_entry = zend_hash_index_find_ptr(ht_list, stream_entry_hash)) != NULL) {
    stream = stream_entry->stream;
    if (stream && stream_entry->can_delete == PPROFILE_CLOSE_LOGGER_STREAM_CAN_DELETE) {
      if (php_stream_stat_path_ex(opt, PHP_STREAM_URL_STAT_QUIET | PHP_STREAM_URL_STAT_NOCACHE, &dest_s, NULL) < 0) {
        goto create_stream;
      }
      return stream;
    }
    return NULL;
  } else {
    create_stream:
    stream = pprofile_stream_open_wrapper(opt TSRMLS_CC);
    if (stream == NULL) {
      return NULL;
    } else {
      stream_entry = ecalloc(1, sizeof(pprofile_stream_entry_t));
      stream_entry->opt_len = (int) spprintf(&stream_entry->opt, 0, "%s", opt);
      stream_entry->stream_entry_hash = stream_entry_hash;
      stream_entry->stream = stream;
      stream_entry->can_delete = PPROFILE_CLOSE_LOGGER_STREAM_CAN_DELETE;

      PPROFILE_ZEND_HASH_INDEX_ADD(ht_list, stream_entry_hash, stream_entry, sizeof(stream_entry_t));
      return stream;
    }
  }
}