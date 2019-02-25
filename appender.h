//
// Created by Tairy on 2019-02-20.
//

#ifndef PPROFILE_APPENDER_H
#define PPROFILE_APPENDER_H

void pprofile_log_ex(zval *log_info TSRMLS_DC);
static int appender_handle_file(char *message, size_t message_len, pprofile_logger_entry_t *logger);

#endif //PPROFILE_APPENDER_H