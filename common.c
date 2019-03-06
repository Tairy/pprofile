//
// Created by Tairy on 2019-03-06.
//

#include "php.h"
#include "php_pprofile.h"
#include "main/php_main.h"

#include "common.h"

int check_sapi_is_cli(TSRMLS_D) {
  if (!strncmp(sapi_module.name, PPROFILE_CLI_KEY, sizeof(PPROFILE_CLI_KEY) - 1)) {
    return SUCCESS;
  }
  return FAILURE;
}
