#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_pprofile.h"

extern ZEND_DECLARE_MODULE_GLOBALS(pprofile);

#include "tracing.h"

static void *(*_zend_malloc)(size_t);
static void (*_zend_free)(void *);
static void *(*_zend_realloc)(void *, size_t);

void *pprofile_malloc(size_t size);
void pprofile_free(void *ptr);
void *pprofile_realloc(void *ptr, size_t size);

zend_always_inline static int tracing_enter_frame_call_graph(zend_string *root_symbol,
                                                             zend_execute_data *execute_data
                                                             TSRMLS_DC);

void tracing_enter_root_frame(TSRMLS_D) {
  PPRG(start_time) = time_milliseconds();
  PPRG(start_timestamp) = current_timestamp();
  PPRG(enabled) = 1;
  PPRG(root) = zend_string_init(PPROFILE_ROOT_SYMBOL, sizeof(PPROFILE_ROOT_SYMBOL) - 1, 0);

  tracing_enter_frame_call_graph(PPRG(root), NULL TSRMLS_CC);
}

void tracing_begin(zend_long flags TSRMLS_CC) {
  int i;

  PPRG(flags) = flags;
  PPRG(call_graph_frames) = NULL;

  for (i = 0; i < PPROFILE_CALL_GRAPH_SLOTS; i++) {
    PPRG(call_graph_buckets)[i] = NULL;
  }

  for (i = 0; i < PPROFILE_CALL_GRAPH_COUNTER_SIZE; i++) {
    PPRG(function_hash_counters)[i] = 0;
  }

  if (flags & PPROFILE_FLAGS_MEMORY_ALLOC) {
    zend_mm_heap *heap = zend_mm_get_heap();
    // hook 内存申请函数，记录内存申请和释放次数
    zend_mm_get_custom_handlers(heap, &_zend_malloc, &_zend_free, &_zend_realloc);
    zend_mm_set_custom_handlers(heap, &pprofile_malloc, &pprofile_free, &pprofile_realloc);
  }
}

void *pprofile_malloc(size_t size) {
  PPRG(num_alloc) += 1;
  PPRG(amount_alloc) += size;

  if (_zend_malloc) {
    return _zend_malloc(size);
  }

  zend_mm_heap *heap = zend_mm_get_heap();
  return zend_mm_alloc(heap, size);
}

void pprofile_free(void *ptr) {
  PPRG(num_free) += 1;

  if (_zend_free) {
    return _zend_free(ptr);
  }

  zend_mm_heap *heap = zend_mm_get_heap();
  return zend_mm_free(heap, ptr);
}

void *pprofile_realloc(void *ptr, size_t size) {
  PPRG(num_alloc) += 1;
  PPRG(num_free) += 1;
  PPRG(amount_alloc) += size;

  if (_zend_realloc) {
    return _zend_realloc(ptr, size);
  }

  zend_mm_heap *heap = zend_mm_get_heap();
  return zend_mm_realloc(heap, ptr, size);
}

zend_always_inline static zend_ulong hash_data(zend_ulong hash, char *data, size_t size) {
  size_t i;

  for (i = 0; i < size; ++i) {
    hash = hash * 33 + data[i];
  }

  return hash;
}

zend_always_inline static zend_ulong hash_int(zend_ulong hash, int data) {
  return hash_data(hash, (char *) &data, sizeof(data));
}

zend_ulong tracing_call_graph_bucket_key(pprofile_frame_t *frame) {
  zend_ulong hash = 5381;

  pprofile_frame_t *previous = frame->prev_frame;

  if (previous) {
    if (previous->class_name) {
      hash = hash_int(hash, ZSTR_HASH(previous->class_name));
    }

    if (previous->function_name) {
      hash = hash_int(hash, ZSTR_HASH(previous->function_name));
    }

    hash += previous->recurse_level;
  }

  if (frame->class_name) {
    hash = hash_int(hash, ZSTR_HASH(frame->class_name));
  }

  if (frame->function_name) {
    hash = hash_int(hash, ZSTR_HASH(frame->function_name));
  }

  hash += frame->recurse_level;

  return hash;
}

pprofile_call_graph_bucket_t *tracing_call_graph_bucket_find(pprofile_call_graph_bucket_t *bucket,
                                                             pprofile_frame_t *current_frame,
                                                             pprofile_frame_t *previous,
                                                             zend_long key) {
  while (bucket) {
    if (bucket->key == key &&
        bucket->child_recurse_level == current_frame->recurse_level &&
        bucket->child_class == current_frame->class_name &&
        bucket->child_function == current_frame->function_name) {

      if (previous == NULL && bucket->parent_class == NULL && bucket->parent_function == NULL) {
        return bucket;
      } else if (previous &&
          previous->recurse_level == bucket->parent_recurse_level &&
          previous->class_name == bucket->parent_class &&
          previous->function_name == bucket->parent_function) {
        return bucket;
      }

      bucket = bucket->next;
    }
  }

  return NULL;
}