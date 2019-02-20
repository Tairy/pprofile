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

#endif //PPROFILE_TRACING_H
