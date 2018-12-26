#ifndef WTK_SEMDLG_CLS_WTK_CLSVEC3
#define WTK_SEMDLG_CLS_WTK_CLSVEC3
#include "wtk/core/wtk_type.h" 
#include "wtk_clsvec_dat.h"
#include "wtk/core/math/wtk_mat.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_clsvec3 wtk_clsvec3_t;

typedef struct
{
	wtk_queue_t item_q;
	wtk_sfvec_t vec;
}wtk_clsvec3_class_t;

struct wtk_clsvec3
{
	wtk_clsvec_dat_t *dat;
	wtk_heap_t *heap;
	wtk_clsvec3_class_t cls;
	double min;
	double min2;
	int iter;
};

wtk_clsvec3_t* wtk_clsvec3_new();
void wtk_clsvec3_delete(wtk_clsvec3_t *v);
void wtk_clsvec3_process(wtk_clsvec3_t *v,char *fn,char *wrd_list);
void wtk_clsvec3_print(wtk_clsvec3_t *v);
#ifdef __cplusplus
};
#endif
#endif
