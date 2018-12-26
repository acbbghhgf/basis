#ifndef WTK_VM_CACHE_WTK_CACHE_H_
#define WTK_VM_CACHE_WTK_CACHE_H_
#include "wtk/core/wtk_sqlite.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk_cache_cfg.h"
#include "wtk_kv.h"
#include "wtk/core/param/wtk_param.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_cache wtk_cache_t;
typedef int (*wtk_cache_param_query_f)(void *data,char *key,int kl,wtk_string_t *v);
typedef int (*wtk_cache_process_f)(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v);
typedef int (*wtk_cache_clear_f)(void *hook);
typedef int (*wtk_cache_set_f)(void *hook,char *k,int kl,char *v,int vl);
typedef int (*wtk_cache_del_f)(void *hook,char *k,int kl);
#define wtk_cache_set_s(c,k,v,update) wtk_cache_set(c,k,sizeof(k)-1,v,sizeof(v)-1,update)
#define wtk_cache_get_s(c,k) wtk_cache_get(c,k,sizeof(k)-1)

struct wtk_cache
{
	wtk_cache_cfg_t *cfg;
	wtk_str_hash_t *hash;
	wtk_str_hash_t *cmd_hash;
	wtk_sqlite_t *sqlite;
	wtk_httpc_t *httpc;
	wtk_queue_t kv_link_queue;  //the header is the last active kv,and the tailer is fresher.
	wtk_cache_set_f set;
	wtk_cache_del_f del;
	wtk_cache_clear_f clear;
	void *hook;
};

wtk_cache_t* wtk_cache_new(wtk_cache_cfg_t *cfg);
int wtk_cache_delete(wtk_cache_t *c);
int wtk_cache_bytes(wtk_cache_t *c);
int wtk_cache_set(wtk_cache_t *c,char *k,int kl,char *v,int vl,int update_httpc);
int wtk_cache_del(wtk_cache_t *c,char *k,int kl,int update_httpc);
int wtk_cache_clear(wtk_cache_t *c,int update_httpc);
wtk_string_t* wtk_cache_get(wtk_cache_t *c,char *k,int kl);
int wtk_cache_show(wtk_cache_t *c,wtk_string_t *v);
int wtk_cache_process(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v);
int wtk_cache_process_clear(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v);
int wtk_cache_process_set(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v);
int wtk_cache_process_del(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v);
int wtk_cache_process_get(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v);
int wtk_cache_process_show(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v);
#ifdef __cplusplus
};
#endif
#endif
