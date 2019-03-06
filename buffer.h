//
// Created by Tairy on 2019-02-27.
//

#ifndef PPROFILE_BUFFER_H
#define PPROFILE_BUFFER_H

void pprofile_init_buffer_switch(TSRMLS_D);
int pprofile_check_buffer_enable(TSRMLS_D);
int pprofile_buffer_set(char *log_info, int log_info_len);

#endif //PPROFILE_BUFFER_H
