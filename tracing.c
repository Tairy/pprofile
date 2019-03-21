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

void tracing_determine_clock_source(TSRMLS_D) {
  struct timespec res;

  if (PPRG(clock_use_rdtsc) == 1) {
    PPRG(clock_source) = PPROFILE_CLOCK_TSC;
  } else if (clock_gettime(CLOCK_MONOTONIC, &res) == 0) {
    PPRG(clock_source) = PPROFILE_CLOCK_CGT;
  } else {
    PPRG(clock_source) = PPROFILE_CLOCK_GTOD;
  }
}

zend_always_inline static zend_ulong hash_data(zend_ulong hash, char *data, size_t size) {
  int i;

  for (i = 0; i < (int) size; ++i) {
    hash = hash * 33 + data[i];
  }

  return hash;
}

zend_always_inline static zend_ulong hash_int(zend_ulong hash, int data) {
  return hash_data(hash, (char *) &data, sizeof(data));
}

void tracing_enter_root_frame(TSRMLS_D) {
  PPRG(start_time) = current_time_milliseconds();
  PPRG(start_timestamp) = current_timestamp();
  PPRG(enabled) = 1;
  PPRG(root) = zend_string_init(PPROFILE_ROOT_SYMBOL, sizeof(PPROFILE_ROOT_SYMBOL) - 1, 0);

  tracing_enter_frame_call_graph(PPRG(root), NULL TSRMLS_CC);
}

void tracing_end(TSRMLS_D) {
  if (PPRG(enabled) == 1) {
    if (PPRG(root)) {
      zend_string_release(PPRG(root));
    }

    while (PPRG(call_graph_frames)) {
      tracing_exit_frame_call_graph(TSRMLS_C);
    }

    PPRG(enabled) = 0;
    PPRG(call_graph_frames) = NULL;

    zend_mm_heap *heap = zend_mm_get_heap();

    if (_zend_malloc || _zend_free || _zend_realloc) {
      zend_mm_set_custom_handlers(heap, _zend_malloc, _zend_free, _zend_realloc);
      _zend_malloc = NULL;
      _zend_free = NULL;
      _zend_realloc = NULL;
    } else {
      *((int *) heap) = 0;
    }
  }
}

zend_always_inline static void tracing_free_the_free_list(TSRMLS_D) {
  pprofile_frame_t *frame = PPRG(frame_free_list);
  pprofile_frame_t *current;

  while (frame) {
    current = frame;
    frame = frame->prev_frame;
    efree(current);
  }
}

void tracing_call_graph_bucket_free(pprofile_call_graph_bucket_t *bucket) {
  if (bucket->parent_class) {
    zend_string_release(bucket->parent_class);
  }

  if (bucket->parent_function) {
    zend_string_release(bucket->parent_function);
  }

  if (bucket->child_class) {
    zend_string_release(bucket->child_class);
  }

  if (bucket->child_function) {
    zend_string_release(bucket->child_function);
  }

  efree(bucket);
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
    }

    bucket = bucket->next;
  }

  return NULL;
}

zend_ulong tracing_call_graph_bucket_key(pprofile_frame_t *frame) {
  zend_ulong hash = 5381;

  pprofile_frame_t *previous = frame->prev_frame;

  if (previous) {
    if (previous->class_name) {
      hash = hash_int(hash, (int) ZSTR_HASH(previous->class_name));
    }

    if (previous->function_name) {
      hash = hash_int(hash, (int) ZSTR_HASH(previous->function_name));
    }

    hash += previous->recurse_level;
  }

  if (frame->class_name) {
    hash = hash_int(hash, (int) ZSTR_HASH(frame->class_name));
  }

  if (frame->function_name) {
    hash = hash_int(hash, (int) ZSTR_HASH(frame->function_name));
  }

  hash += frame->recurse_level;

  return hash;
}

void tracing_call_graph_get_parent_child_name(pprofile_call_graph_bucket_t *bucket,
                                              char *symbol,
                                              size_t symbol_len
                                              TSRMLS_DC) {
  if (bucket->parent_class) {
    if (bucket->parent_recurse_level > 0) {
      snprintf(symbol,
               symbol_len,
               "%s::%s@%d>",
               ZSTR_VAL(bucket->parent_class),
               ZSTR_VAL(bucket->parent_function),
               bucket->parent_recurse_level);
    } else {
      snprintf(symbol, symbol_len, "%s::%s>", ZSTR_VAL(bucket->parent_class), ZSTR_VAL(bucket->parent_function));
    }
  } else if (bucket->parent_function) {
    if (bucket->parent_recurse_level > 0) {
      snprintf(symbol, symbol_len, "%s@%d>", ZSTR_VAL(bucket->parent_function), bucket->parent_recurse_level);
    } else {
      snprintf(symbol, symbol_len, "%s>", ZSTR_VAL(bucket->parent_function));
    }
  } else {
    snprintf(symbol, symbol_len, "");
  }

  if (bucket->child_class) {
    if (bucket->child_recurse_level > 0) {
      snprintf(symbol,
               symbol_len,
               "%s%s::%s@%d",
               symbol,
               ZSTR_VAL(bucket->child_class),
               ZSTR_VAL(bucket->child_function),
               bucket->child_recurse_level);
    } else {
      snprintf(symbol, symbol_len, "%s%s::%s", symbol, ZSTR_VAL(bucket->child_class), ZSTR_VAL(bucket->child_function));
    }
  } else if (bucket->child_function) {
    if (bucket->child_recurse_level > 0) {
      snprintf(symbol, symbol_len, "%s%s@%d", symbol, ZSTR_VAL(bucket->child_function), bucket->child_recurse_level);
    } else {
      snprintf(symbol, symbol_len, "%s%s", symbol, ZSTR_VAL(bucket->child_function));
    }
  }
}

void tracing_call_graph_append_to_array(zval *return_value TSRMLS_DC) {
  int i = 0;

  pprofile_call_graph_bucket_t *bucket;
  char symbol[512] = "";
  zval
  stats_zv, *stats = &stats_zv;

  for (i = 0; i < PPROFILE_CALL_GRAPH_SLOTS; i++) {
    bucket = PPRG(call_graph_buckets)[i];

    while (bucket) {
      tracing_call_graph_get_parent_child_name(bucket, symbol, sizeof(symbol) TSRMLS_CC);

      array_init(stats);
      add_assoc_long(stats, "ct", bucket->count); // 调用次数
      add_assoc_long(stats, "wt", bucket->wall_time); //
      add_assoc_long(stats, "memna", bucket->num_alloc); // 内存分配次数
      add_assoc_long(stats, "memnf", bucket->num_free); // 内存释放次数
      add_assoc_long(stats, "memaa", bucket->amount_alloc); // 分配内存总量
      add_assoc_long(stats, "cpu", bucket->cpu_time); // cpu 耗时
      add_assoc_long(stats, "mu", bucket->memory); // 内存占用
      add_assoc_long(stats, "pmu", bucket->memory_peak); // 内存峰值

      // 这里返回统计信息
      add_assoc_zval(return_value, symbol, stats);

      PPRG(call_graph_buckets)[i] = bucket->next;

      tracing_call_graph_bucket_free(bucket);
      bucket = PPRG(call_graph_buckets)[i];
    }
  }
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

  zend_mm_heap *heap = zend_mm_get_heap();
  // hook 内存申请函数，记录内存申请和释放次数
  zend_mm_get_custom_handlers(heap, &_zend_malloc, &_zend_free, &_zend_realloc);
  zend_mm_set_custom_handlers(heap, &pprofile_malloc, &pprofile_free, &pprofile_realloc);
}

void tracing_request_init(TSRMLS_D) {
  PPRG(timebase_factor) = get_timebase_factor();
  PPRG(enabled) = 0;
  PPRG(flags) = 0;
  PPRG(frame_free_list) = NULL;

  PPRG(num_alloc) = 0;
  PPRG(num_free) = 0;
  PPRG(amount_alloc) = 0;
}

void tracing_request_shutdown() {
  tracing_free_the_free_list(TSRMLS_C);
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