#ifndef WTK_LEX_NER_WTK_HMMNR_CFG
#define WTK_LEX_NER_WTK_HMMNR_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/wtk_str_parser.h"
#include "wtk/core/wtk_fkv.h"
#include "wtk_hmmne.h"
#include "math.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_hmmnr_cfg wtk_hmmnr_cfg_t;

struct wtk_hmmnr_cfg {
    wtk_hmmne_t *ne;
    char *fn;
    char *map_fn;
    int hash_hint;	//3507
};

int wtk_hmmnr_cfg_init(wtk_hmmnr_cfg_t *cfg);
int wtk_hmmnr_cfg_clean(wtk_hmmnr_cfg_t *cfg);
int wtk_hmmnr_cfg_update_local(wtk_hmmnr_cfg_t *cfg, wtk_local_cfg_t *lc);
int wtk_hmmnr_cfg_update(wtk_hmmnr_cfg_t *cfg);
float wtk_hmmnr_cfg_get_prob(wtk_hmmnr_cfg_t *cfg, char *s, int len);
#ifdef __cplusplus
}
;
#endif
#endif
