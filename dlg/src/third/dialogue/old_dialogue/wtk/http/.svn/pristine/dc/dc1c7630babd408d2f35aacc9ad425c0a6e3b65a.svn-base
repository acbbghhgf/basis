#ifndef WTK_VM_CACHE_WTK_KV_H_
#define WTK_VM_CACHE_WTK_KV_H_
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_str_hash.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_kv wtk_kv_t;
struct wtk_kv
{
	hash_str_node_t hash_n;
	wtk_queue_node_t link_n;
	wtk_string_t *k;
	wtk_string_t *v;
};

wtk_kv_t* wtk_kv_new(char *k,int kl,char *v,int vl);
int wtk_kv_delete(wtk_kv_t *kv);
#ifdef __cplusplus
};
#endif
#endif
