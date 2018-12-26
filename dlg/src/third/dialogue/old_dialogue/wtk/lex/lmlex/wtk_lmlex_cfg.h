#ifndef WTK_LMLEX_WTK_LMLEX_CFG_H_
#define WTK_LMLEX_WTK_LMLEX_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/lex/lmlex/lmrec/wtk_lmrec.h"
#include "wtk/lex/lmlex/lmres/wtk_lmres.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lmlex_cfg wtk_lmlex_cfg_t;
struct wtk_lmlex_cfg {
    wtk_lmres_cfg_t lmres;
    wtk_lmrec_cfg_t lmrec;
};

int wtk_lmlex_cfg_init(wtk_lmlex_cfg_t *cfg);
int wtk_lmlex_cfg_clean(wtk_lmlex_cfg_t *cfg);
int wtk_lmlex_cfg_update_local(wtk_lmlex_cfg_t *cfg, wtk_local_cfg_t *lc);
int wtk_lmlex_cfg_update(wtk_lmlex_cfg_t *cfg);
#ifdef __cplusplus
}
;
#endif
#endif
