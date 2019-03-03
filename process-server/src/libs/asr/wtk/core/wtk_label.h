#ifndef WTK_MODEL_WTK_LABEL_H_
#define WTK_MODEL_WTK_LABEL_H_
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/wtk_queue3.h"
#ifdef __cplusplus
extern "C" {
#endif
#define wtk_label_find_s(l,s,i) wtk_label_find(l,s,sizeof(s)-1,i)
typedef struct wtk_label wtk_label_t;

typedef struct wtk_label_node wtk_label_node_t;

struct wtk_label_node
{
	wtk_label_node_t *next;
};

typedef struct
{
	wtk_label_node_t q_n;
	wtk_string_t* name;
	void *data;
}wtk_name_t;

struct wtk_label
{
	wtk_heap_t *heap;
	wtk_label_node_t **slot;
	int nslot;
};

wtk_label_t* wtk_label_new(int hint);
int wtk_label_delete(wtk_label_t *l);
int wtk_label_bytes(wtk_label_t *l);
int wtk_label_init(wtk_label_t* l,int hint);
int wtk_label_clean(wtk_label_t *l);
int wtk_label_reset(wtk_label_t *l);
wtk_name_t* wtk_label_find(wtk_label_t *l,char *s,int sl,int insert);
wtk_string_t* wtk_label_find2(wtk_label_t *l,char *s,int sl,int insert);
#ifdef __cplusplus
};
#endif
#endif
