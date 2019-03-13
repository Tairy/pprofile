//
// Created by Tairy on 2019-03-13.
//

#ifndef PPROFILE_SHM_H
#define PPROFILE_SHM_H

struct shm {
  void *addr;
  size_t size;
};

int shm_alloc(struct shm *shm);
void shm_free(struct shm *shm);

#endif //PPROFILE_SHM_H
