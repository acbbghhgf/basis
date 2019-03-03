#ifndef WTK_CORE_SEGMENTER_WTK_POSDICT
#define WTK_CORE_SEGMENTER_WTK_POSDICT
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/cfg/wtk_source.h"
#include "wtk/core/wtk_fkv2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_posdict wtk_posdict_t;

typedef struct
{
	wtk_string_t wrd;
	wtk_string_t pos;
	float freq;
}wtk_posdict_wrd_t;

struct wtk_posdict
{
	union
	{
		wtk_str_hash_t *map;
		wtk_fkv2_t *kv;
	}v;
	wtk_posdict_wrd_t wrd;
	unsigned use_bin:1;
};

wtk_posdict_t* wtk_posdict_new();
void wtk_posdict_delete(wtk_posdict_t *d);
void wtk_posdict_set_kv(wtk_posdict_t *d,char *fn);
void wtk_posdict_set_kv2(wtk_posdict_t *d,wtk_rbin2_t *rbin,char *fn);
int wtk_posdict_load(wtk_posdict_t *d,wtk_source_t *src);
wtk_posdict_wrd_t* wtk_posdict_get(wtk_posdict_t *d,char *wrd,int wrd_bytes,int insert);
#ifdef __cplusplus
};
#endif
#endif
