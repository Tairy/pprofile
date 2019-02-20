//
// Created by Tairy on 2019-02-20.
//

#ifndef PPROFILE_APPENDER_H
#define PPROFILE_APPENDER_H

zend_always_inline pprofile_log_ex(int argc, char *message, int message_len, zend_class_entry *ce TSRMLS_DC);

static zend_always_inline int appender_handle_udp_tcp(pprofile_logger_entry_t *logger, zend_class_entry *ce TSRMLS_DC);

#endif //PPROFILE_APPENDER_H