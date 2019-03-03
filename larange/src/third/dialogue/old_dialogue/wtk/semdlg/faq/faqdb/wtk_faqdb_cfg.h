#ifndef WTK_SEMDLG_FAQ_FAQDB_WTK_FAQDB_CFG
#define WTK_SEMDLG_FAQ_FAQDB_WTK_FAQDB_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/lex/wrdvec/wtk_vecfaq.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_faqdb_cfg wtk_faqdb_cfg_t;

struct wtk_faqdb_cfg
{
	char *db;
};

int wtk_faqdb_cfg_init(wtk_faqdb_cfg_t *cfg);
int wtk_faqdb_cfg_clean(wtk_faqdb_cfg_t *cfg);
int wtk_faqdb_cfg_update_local(wtk_faqdb_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_faqdb_cfg_update(wtk_faqdb_cfg_t *cfg);

#ifdef __cplusplus
};
#endif
#endif
