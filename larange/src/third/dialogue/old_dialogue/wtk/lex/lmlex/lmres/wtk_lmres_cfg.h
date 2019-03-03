#ifndef WTK_LMLEX_LMRES_WTK_LMRES_CFG_H_
#define WTK_LMLEX_LMRES_WTK_LMRES_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/lex/lmlex/ngram/wtk_ngram.h"
#include "wtk/lex/pool/wtk_lexpool.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lmres_cfg wtk_lmres_cfg_t;
struct wtk_lmres_cfg {
    wtk_ngram_cfg_t ngram;
    wtk_lexpool_cfg_t lexpool;
};

int wtk_lmres_cfg_init(wtk_lmres_cfg_t *cfg);
int wtk_lmres_cfg_clean(wtk_lmres_cfg_t *cfg);
int wtk_lmres_cfg_update_local(wtk_lmres_cfg_t *cfg, wtk_local_cfg_t *lc);
int wtk_lmres_cfg_update(wtk_lmres_cfg_t *cfg);
#ifdef __cplusplus
}
;
#endif
#endif
