#ifndef WTK_CORE_WTK_JSONKV
#define WTK_CORE_WTK_JSONKV
#include "wtk/core/wtk_type.h" 
#include "wtk/core/json/wtk_json_parse.h"
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_jsonkv wtk_jsonkv_t;
#define wtk_jsonkv_set_s(kv,k,a,a_bytes,v,v_bytes) wtk_jsonkv_set(kv,k,sizeof(k)-1,a,a_bytes,v,v_bytes)
#define wtk_jsonkv_set_ss(kv,k,a,v,v_bytes) wtk_jsonkv_set(kv,k,sizeof(k)-1,a,sizeof(a)-1,v,v_bytes)
#define wtk_jsonkv_set_sss(kv,k,a,v) wtk_jsonkv_set(kv,k,sizeof(k)-1,a,sizeof(a)-1,v,sizeof(v)-1)
#define wtk_jsonkv_get_s(kv,k,a,a_bytes) wtk_jsonkv_get(kv,k,sizeof(k)-1,a,a_bytes)
#define wtk_jsonkv_get_ss(kv,k,a) wtk_jsonkv_get(kv,k,sizeof(k)-1,a,sizeof(a)-1)
typedef wtk_string_t (*wtk_jsonkv_get_map_f)(void *ths,char *k,int k_len);
#define wtk_jsonkv_get_json_s(kv,s)  wtk_jsonkv_get_json(kv,s,sizeof(s)-1)

struct wtk_jsonkv
{
	wtk_json_parser_t *json_parser;
	wtk_strbuf_t *dn;
	wtk_strbuf_t *buf;		//used for save json file name
	wtk_strbuf_t *tmp;
	void *get_map_ths;
	wtk_jsonkv_get_map_f get_map;
};

wtk_jsonkv_t* wtk_jsonkv_new(char *dn);
void wtk_jsonkv_delete(wtk_jsonkv_t *kv);
void wtk_jsonkv_reset(wtk_jsonkv_t *kv);
void wtk_jsonkv_set_dn(wtk_jsonkv_t *kv,char *dn);
wtk_string_t wtk_jsonkv_get_dat(wtk_jsonkv_t *kv,char *k,int k_bytes);
wtk_json_t* wtk_jsonkv_get_json(wtk_jsonkv_t *kv,char *k,int k_bytes);
void wtk_jsonkv_save_json(wtk_jsonkv_t *kv,char *k,int k_bytes,wtk_json_t *json);
void wtk_jsonkv_set_dat(wtk_jsonkv_t *kv,char *k,int k_bytes,char *v,int v_bytes);
wtk_string_t wtk_jsonkv_get(wtk_jsonkv_t *kv,char *k,int k_bytes,char *attr,int attr_bytes);
void wtk_jsonkv_set(wtk_jsonkv_t *kv,char *k,int k_bytes,char *attr,int attr_bytes,char *v,int v_bytes);
#ifdef __cplusplus
};
#endif
#endif
