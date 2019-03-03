#ifndef WTK_CORE_WTK_VPOOL2
#define WTK_CORE_WTK_VPOOL2
#include "wtk/core/wtk_type.h" 
#include "wtk_bit_heap.h"
#include "wtk/core/wtk_robin.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_vpool2 wtk_vpool2_t;
struct wtk_vpool2
{
	wtk_bit_heap_t *heap;
	wtk_robin_t *cache;
	int bytes;
};

wtk_vpool2_t* wtk_vpool2_new(int bytes,int max_free);
void wtk_vpool2_delete(wtk_vpool2_t *v);
void wtk_vpool2_reset(wtk_vpool2_t *v);
void* wtk_vpool2_pop(wtk_vpool2_t *v);
void wtk_vpool2_push(wtk_vpool2_t *v,void *p);
int wtk_vpool2_bytes(wtk_vpool2_t *v);
#ifdef __cplusplus
};
#endif
#endif
