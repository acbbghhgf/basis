#ifndef WTK_SEMDLG_CLS_WTK_CLSVEC_DAT
#define WTK_SEMDLG_CLS_WTK_CLSVEC_DAT
#include "wtk/core/wtk_type.h" 
#include "wtk/core/math/wtk_sfvec.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_str_parser.h"
#include "wtk/core/cfg/wtk_source.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_clsvec_dat wtk_clsvec_dat_t;

typedef struct
{
	wtk_queue_node_t q_n;	//thread_q;
	wtk_queue_node_t item_n;//item_qn;
	wtk_queue_node_t list_n;	//used in list
	int index;
	wtk_string_t *name;
	wtk_sfvec_t vec;
}wtk_clsvec2_item_t;

struct wtk_clsvec_dat
{
	wtk_heap_t *heap;
	wtk_string_t **idx_name;
	wtk_queue_t item_q;
	int vsize;
};

wtk_clsvec_dat_t* wtk_clsvec_dat_new(char *fn);
void wtk_clsvec_dat_delete(wtk_clsvec_dat_t *dat);
wtk_clsvec2_item_t* wtk_clsvec_dat_get(wtk_clsvec_dat_t *dat,char *wrd,int wrd_len);
#ifdef __cplusplus
};
#endif
#endif
