#ifndef WTK_SEMDLG_OWLKG_WTK_OWLKV_CFG
#define WTK_SEMDLG_OWLKG_WTK_OWLKV_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/lex/wtk_lex.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_owlkv_cfg wtk_owlkv_cfg_t;

struct wtk_owlkv_cfg
{
	wtk_str_hash_t *hash;
	char *class_dn;
	char *inst_dn;
	char *owl_fn;
	char *class_lex_fn;
	char *rel_lex_fn;
	char *nlg_fn;
	wtk_lex_net_t *net_class;
	wtk_lex_net_t *net_rel;
	wtk_local_cfg_t *lex;
	unsigned use_random:1;
};

int wtk_owlkv_cfg_init(wtk_owlkv_cfg_t *cfg);
int wtk_owlkv_cfg_clean(wtk_owlkv_cfg_t *cfg);
int wtk_owlkv_cfg_update_local(wtk_owlkv_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_owlkv_cfg_update(wtk_owlkv_cfg_t *cfg);
int wtk_owlkv_cfg_update_lex(wtk_owlkv_cfg_t *cfg,wtk_lexc_t *lex);
#ifdef __cplusplus
};
#endif
#endif
