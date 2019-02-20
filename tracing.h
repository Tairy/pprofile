//
// Created by Tairy on 2019-01-31.
//

#ifndef PPROFILE_TRACING_H
#define PPROFILE_TRACING_H

#include "timer.h"

#define PPROFILE_ROOT_SYMBOL "main()"
#define PPROFILE_CALL_GRAPH_COUNTER_SIZE 1024
#define PPROFILE_CALL_GRAPH_SLOTS 8192
#define PPROFILE_FLAGS_CPU 1
#define PPROFILE_FLAGS_MEMORY_MU 2
#define PPROFILE_FLAGS_MEMORY_PMU 4
#define PPROFILE_FLAGS_MEMORY 6
#define PPROFILE_FLAGS_MEMORY_ALLOC 16
#define PPROFILE_FLAGS_MEMORY_ALLOC_AS_MU (32|16)
#define PPROFILE_FLAGS_NO_BUILTINS 8

void tracing_call_graph_append_to_array(zval *return_value TSRMLS_DC);
void tracing_call_graph_get_parent_child_name(pprofile_call_graph_bucket_t *bucket,
                                              char *symbol,
                                              size_t symbol_len
                                              TSRMLS_DC);
zend_ulong tracing_call_graph_bucket_key(pprofile_frame_t *frame);
pprofile_call_graph_bucket_t *tracing_call_graph_bucket_find(pprofile_call_graph_bucket_t *bucket,
                                                           pprofile_frame_t *current_frame,
                                                           pprofile_frame_t *previous,
                                                           zend_long key);
void tracing_call_graph_bucket_free(pprofile_call_graph_bucket_t *bucket);
void tracing_begin(zend_long flags TSRMLS_CC);
void tracing_end(TSRMLS_D);
void tracing_enter_root_frame(TSRMLS_D);
void tracing_request_init(TSRMLS_D);
void tracing_request_shutdown();
void tracing_determine_clock_source();

#define PPRG(v) ZEND_MODULE_GLOBALS_ACCESSOR(pprofile, v)

static zend_always_inline void tracing_fast_free_frame(pprofile_frame_t *p TSRMLS_DC) {
  if (p->function_name != NULL) {
    zend_string_release(p->function_name);
  }

  if (p->class_name != NULL) {
    zend_string_release(p->class_name);
  }

  p->prev_frame = PPRG(frame_free_list);
  PPRG(frame_free_list) = p;
}

static zend_always_inline pprofile_frame_t *tracing_fast_alloc_frame(TSRMLS_D) {
  pprofile_frame_t *p;

  p = PPRG(frame_free_list);

  if (p) {
    PPRG(frame_free_list) = p->prev_frame;
    return p;
  } else {
    return (pprofile_frame_t *) emalloc(sizeof(pprofile_frame_t));
  }
}

static zend_always_inline zend_string *tracing_get_class_name(zend_execute_data *data TSRMLS_DC) {
  zend_function *curr_func;

  if (!data) {
    return NULL;
  }

  curr_func = data->func;

  if (curr_func->common.scope != NULL) {
    zend_string_addref(curr_func->common.scope->name);

    return curr_func->common.scope->name;
  }

  return NULL;
}

static zend_always_inline zend_string *tracing_get_function_name(zend_execute_data *data TSRMLS_DC) {
  zend_function *curr_func;

  if (!data) {
    return NULL;
  }

  curr_func = data->func;

  if (!curr_func->common.function_name) {
    return NULL;
  }

  zend_string_addref(curr_func->common.function_name);

  return curr_func->common.function_name;
}

static zend_always_inline int tracing_enter_frame_call_graph(zend_string *root_symbol,
                                                             zend_execute_data *execute_data
                                                             TSRMLS_DC) {

  zend_string * function_name =
      (root_symbol != NULL) ? zend_string_copy(root_symbol) : tracing_get_function_name(execute_data TSRMLS_CC);
  pprofile_frame_t *current_frame;
  pprofile_frame_t *p;
  int recurse_level = 0;

  if (function_name == NULL) {
    return 0;
  }

  // 当前执行帧
  current_frame = tracing_fast_alloc_frame(TSRMLS_C);
  current_frame->class_name = (root_symbol == NULL) ? tracing_get_class_name(execute_data TSRMLS_CC) : NULL;
  current_frame->function_name = function_name;
  current_frame->prev_frame = PPRG(call_graph_frames);
  current_frame->recurse_level = 0;
  current_frame->wt_start = time_milliseconds();

  if (PPRG(flags) & PPROFILE_FLAGS_CPU) {
    current_frame->cpu_start = cpu_timer();
  }

  if (PPRG(flags) & PPROFILE_FLAGS_MEMORY_PMU) {
    current_frame->pmu_start = zend_memory_peak_usage(0 TSRMLS_CC);
  }

  if (PPRG(flags) & PPROFILE_FLAGS_MEMORY_MU) {
    current_frame->mu_start = zend_memory_usage(0 TSRMLS_CC);
  }

  current_frame->num_alloc = PPRG(num_alloc);
  current_frame->num_free = PPRG(num_free);
  current_frame->amount_alloc = PPRG(amount_alloc);

  current_frame->hash_code = ZSTR_HASH(function_name) % PPROFILE_CALL_GRAPH_COUNTER_SIZE;
  PPRG(call_graph_frames) = current_frame;

  if (PPRG(function_hash_counters)[current_frame->hash_code] > 0) {
    // 这块计算调用栈深度，感觉有优化空间
    for (p = current_frame->prev_frame; p; p = p->prev_frame) {
      if (current_frame->function_name == p->function_name
          && (!current_frame->class_name || current_frame->class_name == p->class_name)) {
        recurse_level = (p->recurse_level) + 1;
        break;
      }
    }
  }

  PPRG(function_hash_counters)[current_frame->hash_code]++;

  current_frame->recurse_level = recurse_level;

  return 1;
}

static zend_always_inline void tracing_exit_frame_call_graph(TSRMLS_D) {
  pprofile_frame_t *current_frame = PPRG(call_graph_frames);
  pprofile_frame_t *previous = current_frame->prev_frame;
  zend_long duration = time_milliseconds() - current_frame->wt_start;

  zend_ulong key = tracing_call_graph_bucket_key(current_frame);
  unsigned int slot = (unsigned int) key % PPROFILE_CALL_GRAPH_SLOTS;
  pprofile_call_graph_bucket_t *bucket = PPRG(call_graph_buckets)[slot];

  bucket = tracing_call_graph_bucket_find(bucket, current_frame, previous, key);

  if (bucket == NULL) {
    bucket = emalloc(sizeof(pprofile_call_graph_bucket_t));
    bucket->key = key;
    bucket->child_class = current_frame->class_name ? zend_string_copy(current_frame->class_name) : NULL;
    bucket->child_function = zend_string_copy(current_frame->function_name);

    if (previous) {
      bucket->parent_class = previous->class_name ? zend_string_copy(current_frame->prev_frame->class_name) : NULL;
      bucket->parent_function = zend_string_copy(previous->function_name);
      bucket->parent_recurse_level = previous->recurse_level;
    } else {
      bucket->parent_class = NULL;
      bucket->parent_function = NULL;
      bucket->parent_recurse_level = 0;
    }

    bucket->count = 0;
    bucket->wall_time = 0;
    bucket->cpu_time = 0;
    bucket->memory = 0;
    bucket->memory_peak = 0;
    bucket->num_free = 0;
    bucket->num_alloc = 0;
    bucket->amount_alloc = 0;
    bucket->child_recurse_level = current_frame->recurse_level;
    bucket->next = PPRG(call_graph_buckets)[slot];

    PPRG(call_graph_buckets)[slot] = bucket;
  }

  bucket->count++;
  bucket->wall_time += duration;

  bucket->num_alloc += PPRG(num_alloc) - current_frame->num_alloc;
  bucket->num_free += PPRG(num_free) - current_frame->num_free;
  bucket->amount_alloc += PPRG(amount_alloc) - current_frame->amount_alloc;

  if (PPRG(flags) & PPROFILE_FLAGS_CPU) {
    bucket->cpu_time += (cpu_timer() - current_frame->cpu_start);
  }

  if (PPRG(flags) & PPROFILE_FLAGS_MEMORY_MU) {
    bucket->memory += (zend_memory_usage(0 TSRMLS_CC) - current_frame->mu_start);
  }

  if (PPRG(flags) & PPROFILE_FLAGS_MEMORY_PMU) {
    bucket->memory_peak += (zend_memory_peak_usage(0 TSRMLS_CC) - current_frame->pmu_start);
  }

  PPRG(function_hash_counters)[current_frame->hash_code]--;

  PPRG(call_graph_frames) = PPRG(call_graph_frames)->prev_frame;
  tracing_fast_free_frame(current_frame TSRMLS_CC);
}

#endif //PPROFILE_TRACING_H
