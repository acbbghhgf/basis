#ifndef WTK_SEMDLG_WRDVEC_WTK_VECFAQ_CFG
#define WTK_SEMDLG_WRDVEC_WTK_VECFAQ_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk_wrdvec.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_vecfaq_cfg wtk_vecfaq_cfg_t;

typedef struct {
    char *fn;
    float thresh;
    float best_thresh;
    wtk_string_t *nm;
} wtk_vecfaq_dat_cfg_t;

struct wtk_vecfaq_cfg {
    wtk_wrdvec_cfg_t wrdvec;
    wtk_vecfaq_dat_cfg_t *map;
    int nmap;
    wtk_vecfaq_dat_cfg_t *dat;
    int ndat;
    unsigned use_share_wrdvec :1;
};

int wtk_vecfaq_cfg_init(wtk_vecfaq_cfg_t *cfg);
int wtk_vecfaq_cfg_clean(wtk_vecfaq_cfg_t *cfg);
int wtk_vecfaq_cfg_update_local(wtk_vecfaq_cfg_t *cfg, wtk_local_cfg_t *lc);
int wtk_vecfaq_cfg_update(wtk_vecfaq_cfg_t *cfg);
int wtk_vecfaq_cfg_update2(wtk_vecfaq_cfg_t *cfg, wtk_source_loader_t *sl);
#ifdef __cplusplus
}
;
#endif
#endif
