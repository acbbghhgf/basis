#ifndef WTK_SEMDLG_WRDVEC_WTK_FAQ2BIN_CFG
#define WTK_SEMDLG_WRDVEC_WTK_FAQ2BIN_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk_wrdvec.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_faq2bin_cfg wtk_faq2bin_cfg_t;

struct wtk_faq2bin_cfg {
    wtk_wrdvec_cfg_t wrdvec;
    int ncls;
    unsigned use_cls :1;
};

int wtk_faq2bin_cfg_init(wtk_faq2bin_cfg_t *cfg);
int wtk_faq2bin_cfg_clean(wtk_faq2bin_cfg_t *cfg);
int wtk_faq2bin_cfg_update_local(wtk_faq2bin_cfg_t *cfg, wtk_local_cfg_t *lc);
int wtk_faq2bin_cfg_update(wtk_faq2bin_cfg_t *cfg);
#ifdef __cplusplus
}
;
#endif
#endif
