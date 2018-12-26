#ifndef WTK_CORE_WTK_ARRAY_LIST_H_
#define WTK_CORE_WTK_ARRAY_LIST_H_
#include "wtk/core/wtk_type.h"
#include "wtk_queue.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_array_list wtk_array_list_t;
typedef struct wtk_array_item wtk_array_item_t;
#define wtk_array_list_last_item(l) data_offset(l->item_queue.push,wtk_array_item_t,q_n)

struct wtk_array_item
{
	wtk_queue_node_t q_n;
	uint32_t nslot;
	void *slot;
};

struct wtk_array_list
{
	wtk_queue_t item_queue;
	uint32_t item_alloc;
	uint32_t elem_size;
	uint32_t len;
};

wtk_array_list_t* wtk_array_list_new(int elem_size,int item_alloc);
int wtk_array_list_delete(wtk_array_list_t *l);
void wtk_array_list_reset(wtk_array_list_t *l);
void wtk_array_list_push(wtk_array_list_t *l,void *data,int n);
void wtk_array_list_write(wtk_array_list_t *l,char *fn);
void wtk_array_list_print(wtk_array_list_t *l);
#ifdef __cplusplus
};
#endif
#endif
