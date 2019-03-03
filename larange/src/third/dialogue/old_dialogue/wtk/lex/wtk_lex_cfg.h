#ifndef WTK_LEX_WTK_LEX_CFG
#define WTK_LEX_WTK_LEX_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/lex/lexc/wtk_lexc.h"
#include "wtk/lex/lexr/wtk_lexr.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lex_cfg wtk_lex_cfg_t;
struct wtk_lex_cfg {
    wtk_lexc_cfg_t lexc;
    wtk_lexr_cfg_t lexr;
};

int wtk_lex_cfg_init(wtk_lex_cfg_t *cfg);
int wtk_lex_cfg_clean(wtk_lex_cfg_t *cfg);
int wtk_lex_cfg_update_local(wtk_lex_cfg_t *cfg, wtk_local_cfg_t *lc);
int wtk_lex_cfg_update(wtk_lex_cfg_t *cfg);
int wtk_lex_cfg_update2(wtk_lex_cfg_t *cfg, wtk_source_loader_t *sl);
#ifdef __cplusplus
}
;
#endif
#endif
