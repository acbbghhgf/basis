#ifndef WTK_OS_WTK_COND
#define WTK_OS_WTK_COND
#include "wtk/os/wtk_thread.h" 
#include "wtk/os/wtk_lock.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_cond wtk_cond_t;
struct wtk_cond
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

void wtk_cond_init(wtk_cond_t *cond);
void wtk_cond_clean(wtk_cond_t *cond);
int wtk_cond_wait(wtk_cond_t *cond,int ms);
int wtk_cond_wake(wtk_cond_t *cond);
#ifdef __cplusplus
};
#endif
#endif
