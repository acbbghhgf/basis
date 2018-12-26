#ifndef WTK_CORE_WTK_SLIST_H_
#define WTK_CORE_WTK_SLIST_H_
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_slist_node wtk_slist_t;
typedef struct wtk_slist_node wtk_slist_node_t;

struct wtk_slist_node
{
	wtk_slist_node_t *prev;
};

typedef struct
{
	wtk_slist_node_t list_n;
	void *hook;
}wtk_slist_item_t;


void wtk_slist_init(wtk_slist_t *list);
void wtk_slist_push(wtk_slist_t *list,wtk_slist_node_t *node);
void wtk_slist_push_front(wtk_slist_t *list,wtk_slist_node_t *node);
int wtk_slist_remove(wtk_slist_t *list,wtk_slist_node_t *node);
wtk_slist_node_t* wtk_slist_pop(wtk_slist_t *list);
int wtk_slist_len(wtk_slist_t *l);

typedef struct
{
	wtk_slist_node_t *pop;
	wtk_slist_node_t *push;
}wtk_slist2_t;

void wtk_slist2_init(wtk_slist2_t *l);
void wtk_slist2_push(wtk_slist2_t *l,wtk_slist_node_t *n);
wtk_slist_node_t* wtk_slist2_pop(wtk_slist2_t *l);
#ifdef __cplusplus
};
#endif
#endif
