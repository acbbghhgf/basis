#ifndef WTK_CORE_SEGMENTER_WTK_CHNPOS
#define WTK_CORE_SEGMENTER_WTK_CHNPOS
#include "wtk/core/wtk_type.h" 
#include "wtk_chnpos_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_chnpos wtk_chnpos_t;

typedef struct wtk_chnpos_pth wtk_chnpos_pth_t;

struct wtk_chnpos_pth
{
	wtk_chnpos_pth_t *prev;
	wtk_string_t *v;
	int pos;
};

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_chnpos_pth_t *pth;
	wtk_chnpos_state_t *state;
	float like;
}wtk_chnpos_inst_t;


struct wtk_chnpos
{
	wtk_chnpos_cfg_t *cfg;
	wtk_prune_t *prune;
	wtk_heap_t *heap;
	wtk_queue_t inst_q;
	wtk_chnpos_inst_t *max_inst;
	int frame;
};

wtk_chnpos_t* wtk_chnpos_new(wtk_chnpos_cfg_t *cfg);
void wtk_chnpos_delete(wtk_chnpos_t *pos);
void wtk_chnpos_reset(wtk_chnpos_t *pos);
int wtk_chnpos_parse(wtk_chnpos_t *pos,wtk_string_t **strs,int n);
void wtk_chnpos_test(wtk_chnpos_t *pos,char *data,int bytes);
#ifdef __cplusplus
};
#endif
#endif
