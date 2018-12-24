#ifndef CORE_QTK_RWLOCK_H_
#define CORE_QTK_RWLOCK_H_

typedef struct qtk_rwlock qtk_rwlock_t;

#ifndef USE_PTHREAD_LOCK

struct qtk_rwlock {
	int write;
	int read;
};


static inline void
qtk_rwlock_init(struct qtk_rwlock *lock) {
	lock->write = 0;
	lock->read = 0;
}


static inline void
qtk_rwlock_rlock(struct qtk_rwlock *lock) {
	for (;;) {
		while(lock->write) {
			__sync_synchronize();
		}
		__sync_add_and_fetch(&lock->read,1);
		if (lock->write) {
			__sync_sub_and_fetch(&lock->read,1);
		} else {
			break;
		}
	}
}


static inline void
qtk_rwlock_wlock(struct qtk_rwlock *lock) {
	while (__sync_lock_test_and_set(&lock->write,1)) {}
	while(lock->read) {
		__sync_synchronize();
	}
}


static inline void
qtk_rwlock_wunlock(struct qtk_rwlock *lock) {
	__sync_lock_release(&lock->write);
}


static inline void
qtk_rwlock_runlock(struct qtk_rwlock *lock) {
	__sync_sub_and_fetch(&lock->read,1);
}

#else

#include <pthread.h>

// only for some platform doesn't have __sync_*
// todo: check the result of pthread api

struct qtk_rwlock {
	pthread_rwlock_t lock;
};


static inline void
qtk_rwlock_init(struct qtk_rwlock *lock) {
	pthread_rwlock_init(&lock->lock, NULL);
}


static inline void
qtk_rwlock_rlock(struct qtk_rwlock *lock) {
	 pthread_rwlock_rdlock(&lock->lock);
}


static inline void
qtk_rwlock_wlock(struct qtk_rwlock *lock) {
	 pthread_rwlock_wrlock(&lock->lock);
}


static inline void
qtk_rwlock_wunlock(struct qtk_rwlock *lock) {
	pthread_rwlock_unlock(&lock->lock);
}


static inline void
qtk_rwlock_runlock(struct qtk_rwlock *lock) {
	pthread_rwlock_unlock(&lock->lock);
}

#endif

#endif
