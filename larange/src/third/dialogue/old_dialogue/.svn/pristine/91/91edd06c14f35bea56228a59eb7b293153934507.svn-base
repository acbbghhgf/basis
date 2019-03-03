#ifndef WTK_LEX_LEXR_WTK_LEXR_CFG
#define WTK_LEX_LEXR_WTK_LEXR_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/lex/wrdvec/wtk_wrdvec.h"
#include "wtk/core/segmenter/wtk_poseg.h"
#include "wtk_lexr_lib.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lexr_cfg wtk_lexr_cfg_t;

struct wtk_lexr_cfg {
    wtk_wrdvec_cfg_t wrdvec;
    wtk_poseg_cfg_t poseg;
    wtk_array_t *filter;
    wtk_lexr_lib_cfg_t lib;
    char *pron_fn;
    unsigned use_share_lib :1;
    unsigned use_wrdvec :1;
    unsigned use_poseg :1;
    unsigned use_share_wrdvec :1;
    unsigned debug :1;
};

int wtk_lexr_cfg_init(wtk_lexr_cfg_t *cfg);
int wtk_lexr_cfg_clean(wtk_lexr_cfg_t *cfg);
int wtk_lexr_cfg_update_local(wtk_lexr_cfg_t *cfg, wtk_local_cfg_t *lc);
int wtk_lexr_cfg_update(wtk_lexr_cfg_t *cfg);
int wtk_lexr_cfg_update2(wtk_lexr_cfg_t *cfg, wtk_source_loader_t *sl);
#ifdef __cplusplus
}
;
#endif
#endif
