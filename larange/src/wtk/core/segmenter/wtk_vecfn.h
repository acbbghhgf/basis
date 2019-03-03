#ifndef WTK_LEX_WRDVEC_WTK_VECFN
#define WTK_LEX_WRDVEC_WTK_VECFN
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/math/wtk_mat.h"
#include "wtk/core/math/wtk_mat.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_vecfn wtk_vecfn_t;

typedef struct
{
	wtk_vecf_t *v;
	unsigned int pos;	//pos file offset	left<pos<right
	unsigned int content_pos;	//content pos
	unsigned int left;	//left node offset
	unsigned int right;	//right node offset
}wtk_vecfn_node_t;

typedef struct
{
	wtk_vecf_t *vec;
	wtk_strbuf_t *buf;
}wtk_vecfn_item_t;

struct wtk_vecfn
{
	FILE *f;
	wtk_vecfn_node_t *root;
	wtk_vecfn_node_t *tmp;
	wtk_vecfn_node_t *left;
	wtk_vecfn_node_t *right;
	wtk_strbuf_t *buf;
	unsigned int free_pos;
	unsigned load:1;
};

wtk_vecfn_t* wtk_vecfn_new(char *fn,int vec_size);
void wtk_vecfn_delete(wtk_vecfn_t *f);
wtk_string_t wtk_vecfn_get(wtk_vecfn_t *f,wtk_vecf_t *v1,float like_thresh,char *q,int q_bytes,char *a,int a_bytes,int add);
void wtk_vecfn_add(wtk_vecfn_t *f,wtk_vecf_t *v,char *q,int q_bytes,char *a,int a_bytes);
#ifdef __cplusplus
};
#endif
#endif
