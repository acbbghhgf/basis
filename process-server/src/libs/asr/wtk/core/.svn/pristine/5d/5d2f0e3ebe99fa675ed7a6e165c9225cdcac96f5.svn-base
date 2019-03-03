#ifndef WTK_CORE_WTK_KDICT
#define WTK_CORE_WTK_KDICT
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/cfg/wtk_source.h"
#include "wtk_strpool.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_kdict wtk_kdict_t;

typedef void* (*wtk_kdict_parse_value_f)(void *ths,wtk_kdict_t *d,wtk_string_t *k,char *v,int v_bytes);

#define wtk_kdict_get_s(d,k) wtk_kdict_get(d,k,sizeof(k)-1)

struct wtk_kdict
{
	wtk_strpool_t *pool;
	wtk_str_hash_t *hash;
	void* parse_ths;
	wtk_kdict_parse_value_f parse_f;
};

wtk_kdict_t* wtk_kdict_new(wtk_strpool_t *pool,int hint,void *ths,wtk_kdict_parse_value_f parse_f);
void wtk_kdict_delete(wtk_kdict_t *d);
int wtk_kdict_bytes(wtk_kdict_t *d);

int wtk_kdict_load(wtk_kdict_t *d,wtk_source_t *src);
int wtk_kdict_load_file(wtk_kdict_t *d,char *fn);

void* wtk_kdict_get(wtk_kdict_t *d,char *k,int k_bytes);
#ifdef __cplusplus
};
#endif
#endif
