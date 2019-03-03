#ifndef WTK_CORE_SEGMENTER_WTK_CHNPOS_MODEL
#define WTK_CORE_SEGMENTER_WTK_CHNPOS_MODEL
#include "wtk/core/wtk_type.h" 
#include "wtk/core/cfg/wtk_source.h"
#include "wtk/core/wtk_strpool.h"
#include "wtk/core/wtk_hash.h"
#include "wtk/core/wtk_robin.h"
#include "wtk/core/wtk_larray.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_chnpos_model wtk_chnpos_model_t;
typedef struct wtk_chnpos_state wtk_chnpos_state_t;
typedef struct wtk_chnpos_wrd wtk_chnpos_wrd_t;

#define MIN_FLOAT -3.14e15

//B(词首),M (词中),E(词尾)和S(单独成词)
typedef enum
{
	WTK_CHNPOS_B=0,
	WTK_CHNPOS_M,
	WTK_CHNPOS_E,
	WTK_CHNPOS_S,
}wtk_chnpos_bmes_t;

typedef struct
{
	wtk_chnpos_state_t *to;
	float prob;
}wtk_chnpos_arc_t;

typedef struct
{
	wtk_string_t *wrd;
	//wtk_chnpos_wrd_t *wrd;
	float prob;
}wtk_chnpos_emit_t;

struct wtk_chnpos_state
{
	//wtk_queue_node_t q_n;
	wtk_chnpos_arc_t *arcs;
	wtk_chnpos_emit_t *emits;
	unsigned short nemit;
	unsigned short narc;
	float start;//start prob
	unsigned char bmes;
	unsigned char pos;
};

struct wtk_chnpos_wrd
{
	wtk_string_t wrd;
	wtk_chnpos_state_t **states;
	int nstate;
};

struct wtk_chnpos_model
{
	wtk_str_hash_t *pos_map;
	int npos;
	//wtk_queue_t state_q;
	wtk_str_hash_t *wrd_map;
	wtk_hash_t *state_map;
	wtk_robin_t *state_robin;
	wtk_larray_t *pos_a;
};

wtk_chnpos_model_t* wtk_chnpos_model_new();
void wtk_chnpos_model_delete(wtk_chnpos_model_t *m);
int wtk_chnpos_model_load(wtk_chnpos_model_t *m,wtk_source_t *src);
wtk_chnpos_wrd_t* wtk_chnpos_model_get_wrd(wtk_chnpos_model_t *m,char *wrd,int wrd_bytes);
wtk_chnpos_wrd_t wtk_chnpos_model_get_wrd2(wtk_chnpos_model_t *m,char *wrd,int wrd_bytes);
float wtk_chnpos_state_get_emit_prob(wtk_chnpos_state_t *state,wtk_string_t *wrd);
float wtk_chnpos_state_get_trans_prob(wtk_chnpos_state_t *state,wtk_chnpos_state_t *nxt);
int wtk_chnpos_wrd_has_state(wtk_chnpos_wrd_t *wrd,wtk_chnpos_state_t *state);
void wtk_chnpos_state_print(wtk_chnpos_model_t *m,wtk_chnpos_state_t *item);
wtk_string_t* wtk_chnpos_model_get_pos_str(wtk_chnpos_model_t *m,int idx);
#ifdef __cplusplus
};
#endif
#endif
