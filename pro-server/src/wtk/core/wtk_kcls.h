#ifndef WTK_CORE_WTK_KCLS
#define WTK_CORE_WTK_KCLS
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_heap.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_kcls wtk_kcls_t;

typedef struct
{
	wtk_queue_node_t q_n;
	int idx;
	float v;
}wtk_kcls_value_t;

struct wtk_kcls
{
	wtk_queue_node_t q_n;
	wtk_queue_t item_q; //wtk_cls_value_t
	float sse;
	float mean;
};

wtk_kcls_t* wtk_kcls_new(wtk_heap_t *heap);
wtk_kcls_value_t* wtk_kcls_value_new(wtk_heap_t *heap,int idx,float f);
void wtk_kcls_print(wtk_kcls_t *cls);
wtk_kcls_t* wtk_kcls_cluster(wtk_kcls_t *cls,wtk_heap_t *heap,float max_sse);

void wtk_kcls_select_int(int *v,int n);
#ifdef __cplusplus
};
#endif
#endif
