//
// Created by Tairy on 2019-02-20.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_pprofile.h"

static int appender_handle_udp_tcp(char *message,
                                   int message_len,
                                   char *level,
                                   int level_int,
                                   pprofile_logger_entry_t *logger,
                                   zend_class_entry *ce TSRMLS_DC) {
//  char *log_info, *log_context, time_RFC3339;
//  int log_len, log_context_len, PRI;
}

