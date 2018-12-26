#ifndef CORE_QTK_SPINLOCK_H_
#define CORE_QTK_SPINLOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

#define QTK_SPIN_INIT(q)        qtk_spinlock_init(&(q)->lock);
#define QTK_SPIN_LOCK(q)        qtk_spinlock_lock(&(q)->lock);
#define QTK_SPIN_UNLOCK(q)      qtk_spinlock_unlock(&(q)->lock);
#define QTK_SPIN_DESTROY(q)     qtk_spinlock_destroy(&(q)->lock);

typedef struct qtk_spinlock qtk_spinlock_t;

#ifndef USE_PTHREAD_LOCK

struct qtk_spinlock {
	int lock;
};

static inline void
qtk_spinlock_init(qtk_spinlock_t *lock) {
	lock->lock = 0;
}

static inline void
qtk_spinlock_lock(qtk_spinlock_t *lock) {
	while (__sync_lock_test_and_set(&lock->lock,1)) {}
}

static inline int
qtk_spinlock_trylock(qtk_spinlock_t *lock) {
	return __sync_lock_test_and_set(&lock->lock,1) == 0;
}

static inline void
qtk_spinlock_unlock(qtk_spinlock_t *lock) {
	__sync_lock_release(&lock->lock);
}

static inline void
qtk_spinlock_destroy(qtk_spinlock_t *lock) {
	(void) lock;
}

#else

#include <pthread.h>

// we use mutex instead of spinlock for some reason
// you can also replace to pthread_spinlock

struct qtk_spinlock {
	pthread_mutex_t lock;
};

static inline void
qtk_spinlock_init(qtk_spinlock_t *lock) {
	pthread_mutex_init(&lock->lock, NULL);
}

static inline void
qtk_spinlock_lock(qtk_spinlock_t *lock) {
	pthread_mutex_lock(&lock->lock);
}

static inline int
qtk_spinlock_trylock(qtk_spinlock_t *lock) {
	return pthread_mutex_trylock(&lock->lock) == 0;
}

static inline void
qtk_spinlock_unlock(qtk_spinlock_t *lock) {
	pthread_mutex_unlock(&lock->lock);
}

static inline void
qtk_spinlock_destroy(qtk_spinlock_t *lock) {
	pthread_mutex_destroy(&lock->lock);
}

#endif

#ifdef __cplusplus
};
#endif

#endif
