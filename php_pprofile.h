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

#define PPROFILE_STREAM_LIST_DESTROY_YES 1

#define PPROFILE_CLOSE_LOGGER_STREAM_MOD_ALL 1
#define PPROFILE_CLOSE_LOGGER_STREAM_MOD_ASSIGN 2
#define PPROFILE_CLOSE_LOGGER_STREAM_CAN_DELETE 3

#define PPROFILE_APPENDER_FILE 1
#define PPROFILE_APPENDER_TCP 2
#define PPROFILE_APPENDER_UDP 3

#define PPROFILE_BUFFER_RE_INIT_YES 1
#define PPROFILE_BUFFER_RE_INIT_NO 2

#define PPROFILE_DIR_MODE (mode_t)0777
#define PPROFILE_FILE_MODE (mode_t)0666

#define PPROFILE_CLI_KEY "cli"

#define PPRG(v) ZEND_MODULE_GLOBALS_ACCESSOR(pprofile, v)

# if defined(ZTS) && defined(COMPILE_DL_PPROFILE)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#if !defined(uint64)
#ifdef __x86_64__
typedef unsigned long uint64;
#else
typedef unsigned long long uint64;
#endif
#endif

#if !defined(uint32)
typedef unsigned int uint32;
#endif

#if !defined(atomic_t)
typedef volatile unsigned int atomic_t;
#endif

#define PPROFILE_ARRAY_DESTROY(arr) \
  do { \
    if (IS_ARRAY == Z_TYPE(arr)) { \
      zval_ptr_dtor(&(arr)); \
      ZVAL_NULL(&(arr)); \
    } \
  } while(0)

#define PPROFILE_ZEND_HASH_INDEX_ADD(ht, h, pData, nDataSize) zend_hash_index_add_ptr(ht, h, pData)
#define PPROFILE_ADD_NEXT_INDEX_STRINGL(a, s, l) add_next_index_stringl(a, s, l)
#define PPROFILE_ADD_ASSOC_ZVAL_EX(z, s, l, zn) add_assoc_zval_ex(&z, s, l, &zn)

typedef struct pprofile_frame_t pprofile_frame_t;
typedef struct pprofile_call_graph_bucket_t pprofile_call_graph_bucket_t;
typedef struct pprofile_logger_entry_t pprofile_logger_entry_t;
typedef struct pprofile_stream_entry_t pprofile_stream_entry_t;
typedef struct pprofile_snowflake_context_t pprofile_snowflake_context_t;
typedef struct pprofile_shm_t pprofile_shm_t;

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
  size_t logger_len;
  char *logger_path;
  int logger_path_len;
  int access;
};

struct pprofile_stream_entry_t {
  char *opt;
  int opt_len;
  ulong stream_entry_hash;
  php_stream *stream;
  int can_delete;
};

/**
 * 雪花算法 上下文
 */
struct pprofile_snowflake_context_t {
  atomic_t lock;

  long sequence;

  uint64 last_ts;
  uint64 data_center_id;
  uint64 worker_id;
  uint64 twepoch;

  unsigned char worker_id_bits;
  unsigned char data_center_id_bits;
  unsigned char sequence_bits;

  int worker_id_shift;
  int data_center_id_shift;
  int timestamp_left_shift;

  int sequence_mask;
};

struct pprofile_shm_t {
  void *addr;
  size_t size;
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
  zval buffer;
  zval logger_list;
  zval stream_list;

  int buffer_size;
  int buffer_count;
  zend_bool use_buffer;
  zend_bool buffer_disabled_in_cli;
  zend_bool enable_buffer_real;
  struct timeval remote_timeout_real;

  int appender;
  int appender_retry;
  char *remote_host;
  char *env;
  char *log_dir;
  int remote_port;
  int remote_timeout;

  // uuid
  pprofile_snowflake_context_t *snowflake_context;
  struct pprofile_shm_t shmem;
  pid_t current_pid;
  int le_atom;
  uint64 data_center_id;
  uint64 worker_id;
  uint64 twepoch;

  // call_id
  int call_id;

ZEND_END_MODULE_GLOBALS(pprofile)

#endif    /* PHP_PPROFILE_H */

