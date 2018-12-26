#ifndef WTK_LMLEX_LMREC_WTK_LMREC_CFG_H_
#define WTK_LMLEX_LMREC_WTK_LMREC_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/lex/lmlex/ngram/wtk_ngram_cfg.h"
#include "wtk/lex/pool/wtk_lexpool.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lmrec_cfg wtk_lmrec_cfg_t;
struct wtk_lmrec_cfg {
    float oov_pen;
    float lex_pen;
    float beam;
    unsigned debug :1;
    unsigned debug_best :1;
    unsigned debug_all :1;
};

int wtk_lmrec_cfg_init(wtk_lmrec_cfg_t *cfg);
int wtk_lmrec_cfg_clean(wtk_lmrec_cfg_t *cfg);
int wtk_lmrec_cfg_update_local(wtk_lmrec_cfg_t *cfg, wtk_local_cfg_t *lc);
int wtk_lmrec_cfg_update(wtk_lmrec_cfg_t *cfg);
#ifdef __cplusplus
}
;
#endif
#endif
