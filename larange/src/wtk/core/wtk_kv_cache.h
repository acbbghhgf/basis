#ifndef WTK_CORE_WTK_KV_CACHE_H_
#define WTK_CORE_WTK_KV_CACHE_H_
#include "wtk/core/wtk_str_hash.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_kv_cache wtk_kv_cache_t;
typedef struct
{
	hash_str_node_t hash_n;
	wtk_queue_node_t link_n;
	wtk_string_t *k;
	wtk_string_t *v;
	unsigned v_is_ref:1;
}wtk_kv_item_t;

struct wtk_kv_cache
{
	wtk_str_hash_t *hash;
	wtk_queue_t kv_link_queue;  //the header is the last active kv,and the tailer is fresher.
	int max_active_slot;
};

//========================= KV ITEM ============================
wtk_kv_item_t* wtk_kv_item_new(char *k,int kl,char *v,int vl);
int wtk_kv_item_delete(wtk_kv_item_t *kv);
//==============================================================

/**
 * @param max_active_slot will keep max cached kv items. if -1, will store all kv, and will not remove oldest item.
 */
wtk_kv_cache_t* wtk_kv_cache_new(int max_active_slot);
int wtk_kv_cache_delete(wtk_kv_cache_t *c);
void wtk_kv_cache_reset(wtk_kv_cache_t *c);

/**
 * @brief add will not check k exist or not, if want safe call wtk_kv_cache_get first, if exist,then not add.
 */
wtk_string_t* wtk_kv_cache_add(wtk_kv_cache_t *c,char *k,int k_bytes,char *v,int v_bytes);
wtk_string_t* wtk_kv_cache_add2(wtk_kv_cache_t *c,char *k,int k_bytes,wtk_string_t *v);
wtk_string_t *wtk_kv_cache_get(wtk_kv_cache_t *c,char *k,int k_bytes);
void wtk_kv_cache_del(wtk_kv_cache_t *c,char *k,int k_bytes);
#ifdef __cplusplus
};
#endif
#endif
