#ifndef WTK_CORE_MATH_WTK_SPARSEM
#define WTK_CORE_MATH_WTK_SPARSEM
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_heap.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_sparsem wtk_sparsem_t;
typedef struct wtk_sparsec wtk_sparsec_t;

struct wtk_sparsec
{
	wtk_sparsec_t *next;
	unsigned int k;
	float v;
};

struct wtk_sparsem
{
	wtk_heap_t *heap;
	wtk_sparsec_t **hash;
	int row;
	int col;
	int hint;

	wtk_sparsec_t *addr;
	int nbytes;
	int nslot;
	int pos;
};


wtk_sparsem_t* wtk_sparsem_new(int row,int col,int hint);
void wtk_sparsem_delete(wtk_sparsem_t *m);
int wtk_sparsem_bytes(wtk_sparsem_t *m);
void wtk_sparsem_set(wtk_sparsem_t *m,int i,int j,float v);
float wtk_sparsem_get(wtk_sparsem_t *m,int i,int j);
#ifdef __cplusplus
};
#endif
#endif
