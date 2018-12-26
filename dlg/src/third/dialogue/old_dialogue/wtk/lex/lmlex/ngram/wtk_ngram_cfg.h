#ifndef WTK_LMLEX_NGRAM_WTK_NGRAM_CFG_H_
#define WTK_LMLEX_NGRAM_WTK_NGRAM_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_ngram_cfg wtk_ngram_cfg_t;
struct wtk_ngram_cfg {
    char *fn;
    wtk_string_t unk;
    wtk_string_t snts;
    wtk_string_t snte;
    wtk_string_t pau;
};

int wtk_ngram_cfg_init(wtk_ngram_cfg_t *cfg);
int wtk_ngram_cfg_clean(wtk_ngram_cfg_t *cfg);
int wtk_ngram_cfg_update_local(wtk_ngram_cfg_t *cfg, wtk_local_cfg_t *lc);
int wtk_ngram_cfg_update(wtk_ngram_cfg_t *cfg);
#ifdef __cplusplus
}
;
#endif
#endif
