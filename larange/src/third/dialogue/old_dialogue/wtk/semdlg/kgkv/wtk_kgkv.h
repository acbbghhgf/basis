#ifndef WTK_SEMDLG_KGKV_WTK_KGKV
#define WTK_SEMDLG_KGKV_WTK_KGKV
#include "wtk/core/wtk_type.h" 
#include "wtk_kgkv_cfg.h"
#include "wtk/core/rbin/wtk_rbin2.h"
#include "wtk/core/wtk_fkv.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_kgkv wtk_kgkv_t;

struct wtk_kgkv
{
	wtk_kgkv_cfg_t *cfg;
	wtk_str_hash_t *hash;
	wtk_fkv_t *fkv;
};

wtk_kgkv_t* wtk_kgkv_new(wtk_kgkv_cfg_t *cfg,wtk_rbin2_t *rbin);
void wtk_kgkv_delete(wtk_kgkv_t *kv);
wtk_string_t wtk_kgkv_get(wtk_kgkv_t *kv,wtk_string_t *db_name,wtk_string_t *k);
#ifdef __cplusplus
};
#endif
#endif
