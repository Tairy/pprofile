//
// Created by Tairy on 2019-03-13.
//

#ifndef PPROFILE_SPINLOCK_H
#define PPROFILE_SPINLOCK_H

void spin_init();
void spin_lock(atomic_t *lock, int id);
void spin_unlock(atomic_t *lock, int id);

#endif //PPROFILE_SPINLOCK_H
