#ifndef WTK_LMLEX_LMRES_WTK_LMRES_H_
#define WTK_LMLEX_LMRES_WTK_LMRES_H_
#include "wtk/core/wtk_type.h"
#include "wtk_lmres_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lmres wtk_lmres_t;

struct wtk_lmres {
    wtk_lmres_cfg_t *cfg;
    wtk_lexpool_t *lexpool;
    wtk_ngram_t *ngram;
};

wtk_lmres_t* wtk_lmres_new(wtk_lmres_cfg_t *cfg);
void wtk_lmres_delete(wtk_lmres_t *res);
void wtk_lmres_reset(wtk_lmres_t *res);
#ifdef __cplusplus
}
;
#endif
#endif
