#ifndef WTK_OS_WTK_RW_LOCK
#define WTK_OS_WTK_RW_LOCK
#include "wtk/core/wtk_type.h" 
#include "wtk_lock.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_rw_lock wtk_rw_lock_t;
struct wtk_rw_lock
{
	wtk_lock_t lock;
	wtk_lock_t xlock;
	volatile unsigned r;
	volatile unsigned w;
};

void wtk_rw_lock_init(wtk_rw_lock_t *lock);
void wtk_rw_lock_clean(wtk_rw_lock_t *lock);
void wtk_rw_lock_lock_read(wtk_rw_lock_t *lock);
void wtk_rw_lock_unlock_read(wtk_rw_lock_t *lock);
void wtk_rw_lock_inc_read(wtk_rw_lock_t *lock);
void wtk_rw_lock_dec_read(wtk_rw_lock_t *lock);
void wtk_rw_lock_start_write(wtk_rw_lock_t *lock);
void wtk_rw_lock_stop_write(wtk_rw_lock_t *lock);

void wtk_rw_lock_start_read(wtk_rw_lock_t *lock);
void wtk_rw_lock_stop_read(wtk_rw_lock_t *lock);
#ifdef __cplusplus
};
#endif
#endif
