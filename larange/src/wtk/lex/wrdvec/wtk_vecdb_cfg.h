#ifndef WTK_LEX_WRDVEC_WTK_VECDB_CFG
#define WTK_LEX_WRDVEC_WTK_VECDB_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk_wrdvec.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_vecdb_cfg wtk_vecdb_cfg_t;
struct wtk_vecdb_cfg {
    wtk_wrdvec_cfg_t wrdvec;
    float get_like_thresh;
    float set_like_thresh;
    int vec_size;
    char *db;
    unsigned use_wrdvec :1;
    unsigned use_share_wrdvec :1;
};

int wtk_vecdb_cfg_init(wtk_vecdb_cfg_t *cfg);
int wtk_vecdb_cfg_clean(wtk_vecdb_cfg_t *cfg);
int wtk_vecdb_cfg_update_local(wtk_vecdb_cfg_t *cfg, wtk_local_cfg_t *lc);
int wtk_vecdb_cfg_update(wtk_vecdb_cfg_t *cfg);
int wtk_vecdb_cfg_update2(wtk_vecdb_cfg_t *cfg, wtk_source_loader_t *sl);
#ifdef __cplusplus
}
;
#endif
#endif
