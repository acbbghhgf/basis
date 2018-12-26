#ifndef WTK_LEX_WRDVEC_WTK_CLSVEC
#define WTK_LEX_WRDVEC_WTK_CLSVEC
#include "wtk/core/wtk_type.h" 
#include "wtk/core/math/wtk_mat.h"
#include "wtk/core/cfg/wtk_source.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_os.h"
#include "wtk_clsvec_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_clsvec wtk_clsvec_t;


typedef struct
{
	wtk_queue_node_t q_n;
	wtk_vecf_t *v;
	wtk_string_t *str;
}wtk_clsvec_item_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_queue_t item_q;
	wtk_vecf_t *v;
	double sse;
}wtk_clsvec_cls_item_t;

struct wtk_clsvec
{
	wtk_clsvec_cfg_t *cfg;
	wtk_string_t **idx_name;
	wtk_heap_t *glb_heap;
	wtk_heap_t *heap;
	wtk_queue_t cls_q;
	wtk_queue_t item_q;
	wtk_queue_t output_q;
	int vsize;
	float sse_thresh;
};

wtk_clsvec_t* wtk_clsvec_new(wtk_clsvec_cfg_t *cfg);
void wtk_clsvec_delete(wtk_clsvec_t *v);
void wtk_clsvec_process(wtk_clsvec_t *v,char *fn);
void wtk_clsvec_print(wtk_clsvec_t *v);


#ifdef __cplusplus
};
#endif
#endif
