//
// Created by Tairy on 2019-02-20.
//

#ifndef PPROFILE_APPENDER_H
#define PPROFILE_APPENDER_H

void pprofile_log_ex(zval *log_info TSRMLS_DC);
int appender_handle_file();

//static zend_always_inline int appender_handle_udp_tcp(pprofile_logger_entry_t *logger, zend_class_entry *ce TSRMLS_DC);

#endif //PPROFILE_APPENDER_H