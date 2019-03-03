#ifndef WTK_OS_WTK_LOCKVPOOL
#define WTK_OS_WTK_LOCKVPOOL
#include "wtk/core/wtk_vpool.h" 
#include "wtk_lock.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lockvpool wtk_lockvpool_t;
struct wtk_lockvpool
{
	wtk_lock_t lock;
	wtk_vpool_t *pool;
};


wtk_lockvpool_t* wtk_lockvpool_new(int bytes,int max_free);
wtk_lockvpool_t* wtk_lockvpool_new2(int bytes,int max_free,int reset_free);
void wtk_lockvpool_delete(wtk_lockvpool_t *v);
void wtk_lockvpool_reset(wtk_lockvpool_t *v);
void* wtk_lockvpool_pop(wtk_lockvpool_t *v);
void wtk_lockvpool_push(wtk_lockvpool_t *v,void *usr_data);
#ifdef __cplusplus
};
#endif
#endif
