#ifndef WTK_OS_WTK_SEM_H_
#define WTK_OS_WTK_SEM_H_
#include "wtk/core/wtk_type.h"
//#define __USE_XOPEN2K
//#define _XOPEN_SOURCE  600
#ifdef __cplusplus
extern "C" {
#endif
#ifndef WIN32
typedef sem_t wtk_sem_t;
#define wtk_sem_init(s,c) sem_init(s,0,c)
#define wtk_sem_clean(s) sem_destroy(s)
#define wtk_sem_tryacquire(s) sem_trywait(s)
#define wtk_sem_inc(s) sem_post(s)

#ifdef USE_ARM
#else
/**
 * @brief create semaphore for shared access; and can be used by processes;
 * 		* sem_init;
 * 		* sem_open;
 */
wtk_sem_t* wtk_sem_new_shared();

/**
 * @brief delete shared semaphore;
 */
void wtk_sem_delete_shared(wtk_sem_t *s);
#endif

#else
typedef HANDLE wtk_sem_t;
int wtk_sem_init(wtk_sem_t *s,int init_count);
int wtk_sem_clean(wtk_sem_t *s);
#define wtk_sem_inc(s) wtk_sem_release(s,1)
#endif

int wtk_sem_acquire(wtk_sem_t *s,int ms);
int wtk_sem_release(wtk_sem_t *s, int count);
int wtk_sem_drain(wtk_sem_t *s);
#ifdef __cplusplus
};
#endif
#endif
