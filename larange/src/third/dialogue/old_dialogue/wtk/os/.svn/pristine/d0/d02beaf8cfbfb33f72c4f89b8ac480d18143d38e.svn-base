#ifndef WTK_OS_WTK_LOCK_H_
#define WTK_OS_WTK_LOCK_H_
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif
#ifndef WIN32
typedef pthread_mutex_t wtk_lock_t;
#define wtk_lock_init(l) pthread_mutex_init(l,0)
#define wtk_lock_clean(l) pthread_mutex_destroy(l)
#define wtk_lock_lock(l) pthread_mutex_lock(l)
#define wtk_lock_unlock(l) pthread_mutex_unlock(l)
#define wtk_lock_trylock(l) pthread_mutex_trylock(l)
//int wtk_lock_init2(wtk_lock_t *lock);
#else
#include <windows.h>
typedef CRITICAL_SECTION wtk_lock_t;
/*
#define wtk_lock_init(l)   (InitializeCriticalSection(l),0)
#define wtk_lock_clean(l)   DeleteCriticalSection(l),0
#define wtk_lock_lock(l)   EnterCriticalSection(l),0
#define wtk_lock_unlock(l)  LeaveCriticalSection(l),0
#define wtk_lock_trylock(l)   TryEnterCriticalSection(l)
*/
int wtk_lock_init(wtk_lock_t *l);
int wtk_lock_clean(wtk_lock_t *l);
int wtk_lock_lock(wtk_lock_t *l);
int wtk_lock_unlock(wtk_lock_t *l);
int wtk_lock_trylock(wtk_lock_t *l);
#endif
#ifdef __cplusplus
};
#endif
#endif
