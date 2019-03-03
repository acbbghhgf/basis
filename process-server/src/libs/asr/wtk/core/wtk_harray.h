#ifndef WTK_CORE_WTK_HARRAY_H_
#define WTK_CORE_WTK_HARRAY_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_heap.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_harray wtk_harray_t;

struct wtk_harray
{
	wtk_heap_t *heap;
	void **slot;
	int nslot1;
	int nslot2;
};

wtk_harray_t* wtk_harray_new(int nslot1,int nslot2);
void wtk_harray_delete(wtk_harray_t *h);
int wtk_harray_bytes(wtk_harray_t *h);
void wtk_harray_reset(wtk_harray_t *h);
void* wtk_harray_get(wtk_harray_t *h,unsigned long idx);
void wtk_harray_set(wtk_harray_t *h,unsigned long idx,void *p);
#ifdef __cplusplus
};
#endif
#endif
