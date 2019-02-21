/* pprofile extension for PHP */

#ifndef PHP_PPROFILE_H
#define PHP_PPROFILE_H

extern zend_module_entry pprofile_module_entry;
#define phpext_pprofile_ptr &pprofile_module_entry

#define PHP_PPROFILE_VERSION "0.1.0"
#define PPROFILE_CALL_GRAPH_COUNTER_SIZE 1024
#define PPROFILE_CALL_GRAPH_SLOTS 8192
#define PPROFILE_CLOCK_CGT 0
#define PPROFILE_CLOCK_GTOD 1
#define PPROFILE_CLOCK_TSC 2
#define PPROFILE_CLOCK_MACH 3
#define PPROFILE_CLOCK_QPC 4
#define PPROFILE_CLOCK_NONE 255

#define PPRG(v) ZEND_MODULE_GLOBALS_ACCESSOR(pprofile, v)

# if defined(ZTS) && defined(COMPILE_DL_PPROFILE)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#if !defined(uint64)
typedef unsigned long long uint64;
#endif

#if !defined(uint32)
typedef unsigned int uint32;
#endif

# define PPROFILE_ARRAY_DESTROY(arr) \
  do { \
    if (IS_ARRAY == Z_TYPE(arr)) { \
      zval_ptr_dtor(&(arr)); \
      ZVAL_NULL(&(arr)); \
    } \
  } while(0)

typedef struct pprofile_frame_t pprofile_frame_t;
typedef struct pprofile_call_graph_bucket_t pprofile_call_graph_bucket_t;
typedef struct pprofile_logger_entry_t pprofile_logger_entry_t;

struct pprofile_call_graph_bucket_t {
  zend_ulong key;
  zend_string *parent_class;
  zend_string *parent_function;
  int parent_recurse_level;
  zend_string *child_class;
  zend_string *child_function;
  int child_recurse_level;
  pprofile_call_graph_bucket_t *next;
  zend_long count;
  zend_long wall_time;
  zend_long cpu_time;
  zend_long memory;
  zend_long memory_peak;
  long int num_alloc, num_free;
  long int amount_alloc;
};

/**
 * 运行时帧结构体
 */
struct pprofile_frame_t {
  struct pprofile_frame_t *prev_frame; // 上一帧
  zend_string *function_name; // 函数名称
  zend_string *class_name; // 类名称
  uint64 wt_start;
  uint64 cpu_start;
  long int mu_start;
  long int pmu_start;
  long int num_alloc, num_free;
  long int amount_alloc;
  int recurse_level;
  zend_ulong hash_code;
};

struct pprofile_logger_entry_t {
  ulong logger_hash;
  char *folder;
  char *logger;
  int logger_len;
  char *logger_path;
  int logger_path_len;
  int access;
};

ZEND_BEGIN_MODULE_GLOBALS(pprofile)
  int enabled;
  zend_long flags;
  uint64 start_timestamp;
  uint64 start_time;
  int clock_source;
  zend_bool clock_use_rdtsc;
  double timebase_factor;
  zend_string *root;
  pprofile_frame_t *call_graph_frames;
  pprofile_frame_t *frame_free_list;
  zend_ulong function_hash_counters[PPROFILE_CALL_GRAPH_COUNTER_SIZE];
  pprofile_call_graph_bucket_t *call_graph_buckets[PPROFILE_CALL_GRAPH_SLOTS];
  long int num_alloc;
  long int num_free;
  long int amount_alloc;

  pprofile_logger_entry_t *last_logger;
  zval logger_list;
ZEND_END_MODULE_GLOBALS(pprofile)

#endif    /* PHP_PPROFILE_H */

