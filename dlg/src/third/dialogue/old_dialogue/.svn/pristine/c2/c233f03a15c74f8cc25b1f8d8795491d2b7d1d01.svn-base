#ifndef WTK_CORE_WTK_QARRAY_H_
#define WTK_CORE_WTK_QARRAY_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_queue.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_qarray wtk_qarray_t;

typedef struct
{
	wtk_queue_node_t q_n;
	union{
		int i;
		float v;
		void *p;
	}v;
	int idx;
}wtk_qarray_item_t;

struct wtk_qarray
{
	wtk_heap_t *heap;
	wtk_queue_t q;	//wtk_qarray_item_t;
	unsigned use_sort:1;
};

wtk_qarray_t* wtk_qarray_new(wtk_heap_t *heap,int use_sort);
wtk_qarray_item_t* wtk_qarray_find(wtk_qarray_t *a,int idx);
wtk_qarray_item_t* wtk_qarray_add_item(wtk_qarray_t *a,int idx);
void wtk_qarray_add_p(wtk_qarray_t *a,int idx,void *ths);
void wtk_qarray_append_p(wtk_qarray_t *a,void *ths);
void* wtk_qarray_get_p(wtk_qarray_t *a,int idx);

#ifdef __cplusplus
};
#endif
#endif
