#ifndef WTK_CORE_WTK_FKV2
#define WTK_CORE_WTK_FKV2
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/wtk_str_encode.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/rbin/wtk_rbin2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_fkv2 wtk_fkv2_t;
#define wtk_fkv2_get_s(kv,env,k) wtk_fkv2_get(kv,env,k,sizeof(k)-1)
#define wtk_fkv2_has_s(kv,env,k) wtk_fkv2_has(kv,env,k,sizeof(k)-1)

/**
 * python kv2bin2.py
 */

typedef enum
{
	WTK_FKV2_INT=1,
	WTK_FKV2_FLOAT,
	WTK_FKV2_STRING,
	//WTK_FKV2_LSTRING,
}wtk_fkv2_type_t;

typedef struct
{
	union {
		int v;
		float f;
		wtk_string_t str;
	}v;
	unsigned int offset;
	unsigned short depth;
	unsigned short is_end:1;
	unsigned short is_err:1;
}wtk_fkv_env_t;

struct wtk_fkv2
{
	FILE *f;
	wtk_str_hash_t *char_map;
	unsigned int max;
	unsigned int step;
	unsigned int nslot;
	unsigned int *slots;
	unsigned int slot_offset;
	wtk_strbuf_t *buf;
	wtk_fkv2_type_t type;
	int f_of;
	int f_len;
	unsigned file_want_close:1;
};

wtk_fkv2_t* wtk_fkv2_new2(wtk_rbin2_t *rbin,char *fn);
wtk_fkv2_t* wtk_fkv2_new(char *fn);
void wtk_fkv2_delete(wtk_fkv2_t* k);
void wtk_fkv_env_init(wtk_fkv2_t *k,wtk_fkv_env_t *env);
void wtk_fkv_env_print(wtk_fkv_env_t *env);
int wtk_fkv2_get(wtk_fkv2_t *kv,wtk_fkv_env_t *env,char *k,int k_bytes);
int wtk_fkv2_has(wtk_fkv2_t *kv,wtk_fkv_env_t *env,char *data,int bytes);
#ifdef __cplusplus
};
#endif
#endif
