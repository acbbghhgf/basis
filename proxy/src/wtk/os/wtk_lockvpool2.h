#ifndef WTK_OS_WTK_lockvpool22
#define WTK_OS_WTK_lockvpool22
#include "wtk/core/wtk_vpool2.h" 
#include "wtk_lock.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lockvpool2 wtk_lockvpool2_t;
struct wtk_lockvpool2
{
	wtk_lock_t lock;
	wtk_vpool2_t *pool;
};

wtk_lockvpool2_t* wtk_lockvpool2_new(int bytes,int max_free);
void wtk_lockvpool2_delete(wtk_lockvpool2_t *v);
void wtk_lockvpool2_reset(wtk_lockvpool2_t *v);
void* wtk_lockvpool2_pop(wtk_lockvpool2_t *v);
void wtk_lockvpool2_push(wtk_lockvpool2_t *v,void *usr_data);

#ifdef __cplusplus
};
#endif
#endif
