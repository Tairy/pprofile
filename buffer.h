//
// Created by Tairy on 2019-02-27.
//

#ifndef PPROFILE_BUFFER_H
#define PPROFILE_BUFFER_H

void pprofile_init_buffer_switch(TSRMLS_D);
int pprofile_check_buffer_enable(TSRMLS_D);
int pprofile_buffer_set(char *log_info, size_t log_info_len, char *path, size_t path_len);
void pprofile_shutdown_buffer(int re_init TSRMLS_DC);

#endif //PPROFILE_BUFFER_H
