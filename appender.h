//
// Created by Tairy on 2019-02-20.
//

#ifndef PPROFILE_APPENDER_H
#define PPROFILE_APPENDER_H

static int appender_handle_udp_tcp(char *message,
                                   int message_len,
                                   char *level,
                                   int level_int,
                                   pprofile_logger_entry_t *logger,
                                   zend_class_entry *ce TSRMLS_DC);

#endif //PPROFILE_APPENDER_H