//
// Created by Tairy on 2019-02-22.
//

#ifndef PPROFILE_STREAM_WRAPPER_H
#define PPROFILE_STREAM_WRAPPER_H

void pprofile_init_stream_list(TSRMLS_D);
int pprofile_free_stream(int destroy, int model, char *opt TSRMLS_DC);
php_stream *precess_stream(char *opt, size_t opt_len TSRMLS_DC);

#endif //PPROFILE_STREAM_WRAPPER_H
