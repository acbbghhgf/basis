#ifndef WTK_CORE_WTK_STRDICT
#define WTK_CORE_WTK_STRDICT
#include "wtk/core/wtk_type.h"
#include "wtk/core/cfg/wtk_source.h"
#include "wtk/core/wtk_str_hash.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_strdict wtk_strdict_t;

typedef struct
{
	wtk_string_t **phns;
	int nph;
}wtk_strdict_phn_t;

struct wtk_strdict
{
	wtk_str_hash_t *hash;
};

wtk_strdict_t* wtk_strdict_new(int hash_hint);
void wtk_strdict_delete(wtk_strdict_t *d);
void wtk_strdict_reset(wtk_strdict_t *d);
int wtk_strdict_load(wtk_strdict_t *d,wtk_source_t *src);

wtk_strdict_phn_t* wtk_strdict_get(wtk_strdict_t *d,char *k,int k_bytes);
#ifdef __cplusplus
};
#endif
#endif
