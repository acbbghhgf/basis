#ifndef WTK_CORE_WTK_VPOOL_H_
#define WTK_CORE_WTK_VPOOL_H_
#include "wtk/core/wtk_bit_heap.h"
#include "wtk/core/wtk_hoard.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_robin.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_vpool wtk_vpool_t;

typedef enum
{
	WTK_VPOOL_BITHEAP,
	WTK_VPOOL_HEAP,
	WTK_VPOOL_CHEAP,
}wtk_vpool_type_t;

struct wtk_vpool
{
	union
	{
		wtk_heap_t *heap;
		wtk_bit_heap_t *bitheap;
	}v;
	wtk_hoard_t hoard;
	int bytes;
	int max;
	int alloc;
	int reset_free;
	wtk_vpool_type_t type;
	wtk_robin_t *cache;
};

wtk_vpool_t* wtk_vpool_new(int bytes,int max_free);
wtk_vpool_t* wtk_vpool_new2(int bytes,int max_free,int reset_free);
wtk_vpool_t* wtk_vpool_new3(int bytes,int max_free,int reset_free,wtk_vpool_type_t type);
wtk_vpool_t* wtk_vpool_new4(int bytes,int max_free,int reset_free,wtk_vpool_type_t type,int max_item);
int wtk_vpool_bytes(wtk_vpool_t *v);
void wtk_vpool_delete(wtk_vpool_t *v);
void wtk_vpool_reset(wtk_vpool_t *v);
void* wtk_vpool_pop(wtk_vpool_t *v);
void wtk_vpool_push(wtk_vpool_t *v,void *usr_data);
#ifdef __cplusplus
};
#endif
#endif
