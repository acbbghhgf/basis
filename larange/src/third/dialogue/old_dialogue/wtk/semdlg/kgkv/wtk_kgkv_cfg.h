#ifndef WTK_SEMDLG_KGKV_WTK_KGKV_CFG
#define WTK_SEMDLG_KGKV_WTK_KGKV_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/core/cfg/wtk_source.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_kgkv_cfg wtk_kgkv_cfg_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_string_t *k;
	char *fn;
}wtk_kgkv_item_t;


struct wtk_kgkv_cfg
{
	char *kv_fn;
	wtk_queue_t item_q;
};

int wtk_kgkv_cfg_init(wtk_kgkv_cfg_t *cfg);
int wtk_kgkv_cfg_clean(wtk_kgkv_cfg_t *cfg);
int wtk_kgkv_cfg_update_local(wtk_kgkv_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_kgkv_cfg_update(wtk_kgkv_cfg_t *cfg);
int wtk_kgkv_cfg_update2(wtk_kgkv_cfg_t *cfg,wtk_source_loader_t *sl);
#ifdef __cplusplus
};
#endif
#endif
