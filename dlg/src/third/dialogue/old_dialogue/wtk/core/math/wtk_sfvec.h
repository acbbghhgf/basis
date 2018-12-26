#ifndef WTK_CORE_MATH_WTK_SFVEC
#define WTK_CORE_MATH_WTK_SFVEC
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_queue3.h"
#include "wtk/core/wtk_heap.h"
#include <math.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_sfvec wtk_sfvec_t;

typedef struct
{
	wtk_queue_node_t q_n;
	int i;
	float v;
}wtk_svec_item_t;

struct wtk_sfvec
{
	wtk_queue3_t item_q;
};

void wtk_sfvec_init(wtk_sfvec_t *v);
void wtk_sfvec_add_value(wtk_sfvec_t *v,wtk_svec_item_t *item);
void wtk_sfvec_print(wtk_sfvec_t *v);
void wtk_sfvec_norm(wtk_sfvec_t *v);
double wtk_sfvec_dist(wtk_sfvec_t *sf1,wtk_sfvec_t *sf);
void wtk_sfvec_dup(wtk_sfvec_t *src,wtk_sfvec_t *dst,wtk_heap_t *heap);
#ifdef __cplusplus
};
#endif
#endif
