#ifndef WTK_CORE_CFG_WTK_CFG_QUEUE_H_
#define WTK_CORE_CFG_WTK_CFG_QUEUE_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_array.h"
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_cfg_queue wtk_cfg_queue_t;
struct wtk_local_cfg;
#define wtk_cfg_item_is_string(cfg) ((cfg)->type==WTK_CFG_STRING)
#define wtk_cfg_item_is_lc(cfg) ((cfg)->type==WTK_CFG_LC)
#define wtk_cfg_item_is_array(cfg) ((cfg)->type==WTK_CFG_ARRAY)
#define wtk_cfg_queue_find_s(c,k) wtk_cfg_queue_find(c,k,sizeof(k)-1)

typedef enum
{
	WTK_CFG_STRING=0,
	WTK_CFG_LC,
	WTK_CFG_ARRAY,
}wtk_cfg_type_t;

typedef struct
{
	wtk_queue_node_t n;
	wtk_cfg_type_t type;
	wtk_string_t *key;
	union
	{
		wtk_string_t *str;
		struct wtk_local_cfg *cfg;
		wtk_array_t *array;	//wtk_string_t* array.
	}value;
}wtk_cfg_item_t;

struct wtk_cfg_queue
{
	wtk_queue_t queue;		//wtk_cfg_item_t
	wtk_heap_t *heap;
};

wtk_cfg_queue_t* wtk_cfg_queue_new_h(wtk_heap_t *heap);
int wtk_cfg_queue_add(wtk_cfg_queue_t *c,wtk_cfg_item_t *item);
wtk_cfg_item_t* wtk_cfg_queue_find(wtk_cfg_queue_t *c,char *k,int bytes);
void wtk_cfg_queue_remove(wtk_cfg_queue_t *c,wtk_cfg_item_t *item);
wtk_cfg_item_t* wtk_cfg_queue_add_string(wtk_cfg_queue_t *cfg,char *k,int kbytes,char *v,int vbytes);
wtk_cfg_item_t* wtk_cfg_queue_add_lc(wtk_cfg_queue_t *cfg,char *k,int kbytes,struct wtk_local_cfg *lc);
wtk_cfg_item_t* wtk_cfg_queue_add_array(wtk_cfg_queue_t *cfg,char *k,int bytes,wtk_array_t *a);

void wtk_cfg_item_print(wtk_cfg_item_t *i);
void wtk_cfg_queue_print(wtk_cfg_queue_t *c);
void wtk_cfg_queue_to_string(wtk_cfg_queue_t *c,wtk_strbuf_t *buf);
#ifdef __cplusplus
};
#endif
#endif
