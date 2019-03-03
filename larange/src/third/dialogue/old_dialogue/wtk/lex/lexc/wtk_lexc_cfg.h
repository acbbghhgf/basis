#ifndef WTK_LEX_LEXC_WTK_LEXC_CFG
#define WTK_LEX_LEXC_WTK_LEXC_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk_lex_ner_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lexc_cfg wtk_lexc_cfg_t;
struct wtk_lexc_cfg {
    wtk_lex_ner_cfg_t ner;
    wtk_array_t *include_path;
    float var_miss_pen;
    unsigned use_eng_word :1;
    unsigned use_act :1;
};

int wtk_lexc_cfg_init(wtk_lexc_cfg_t *cfg);
int wtk_lexc_cfg_clean(wtk_lexc_cfg_t *cfg);
int wtk_lexc_cfg_update_local(wtk_lexc_cfg_t *cfg, wtk_local_cfg_t *lc);
int wtk_lexc_cfg_update(wtk_lexc_cfg_t *cfg);
int wtk_lexc_cfg_update2(wtk_lexc_cfg_t *cfg, wtk_source_loader_t *sl);
#ifdef __cplusplus
}
;
#endif
#endif
