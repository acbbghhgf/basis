#ifndef WTK_LEX_LEXR_WTK_LEX_NER_CFG
#define WTK_LEX_LEXR_WTK_LEX_NER_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/lex/ner/wtk_hmmnr.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lex_ner_cfg wtk_lex_ner_cfg_t;

typedef struct {
    wtk_string_t *name;
    float wrd_pen;
    float prune_thresh;
    float conf_thresh;
    int hash_hint;
    char *fn;
    char *map;
    wtk_fkv_t *fkv;
    wtk_hmmne_t *ne;
} wtk_lexr_ner_item_t;

struct wtk_lex_ner_cfg {
    wtk_str_hash_t *hash;
    int n;
};

int wtk_lex_ner_cfg_init(wtk_lex_ner_cfg_t *cfg);
int wtk_lex_ner_cfg_clean(wtk_lex_ner_cfg_t *cfg);
int wtk_lex_ner_cfg_update_local(wtk_lex_ner_cfg_t *cfg, wtk_local_cfg_t *lc);
int wtk_lex_ner_cfg_update(wtk_lex_ner_cfg_t *cfg);
int wtk_lex_ner_cfg_update2(wtk_lex_ner_cfg_t *cfg, wtk_source_loader_t *sl);
#ifdef __cplusplus
}
;
#endif
#endif
