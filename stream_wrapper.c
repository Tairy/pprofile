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

php_stream *precess_stream(char *opt, size_t opt_len TSRMLS_DC) {
  ulong stream_entry_hash;
  php_stream * stream = NULL;
  HashTable *ht_list;
  php_stream_statbuf dest_s;
  pprofile_stream_entry_t *stream_entry;

  stream_entry_hash = zend_inline_hash_func(opt, opt_len);

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

        }
    ZEND_HASH_FOREACH_END();
  }

  return result;
}