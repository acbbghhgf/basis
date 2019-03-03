#ifndef WTK_SEMDLG_SEMFLD_WTK_SEMFI_CFG
#define WTK_SEMDLG_SEMFLD_WTK_SEMFI_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/lex/wtk_lex.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_semfi_cfg wtk_semfi_cfg_t;
struct wtk_semfi_cfg
{
	char *crffn;
	char *lexfn;
	wtk_lex_net_t *net;
};

int wtk_semfi_cfg_init(wtk_semfi_cfg_t *cfg);
int wtk_semfi_cfg_clean(wtk_semfi_cfg_t *cfg);
int wtk_semfi_cfg_update_local(wtk_semfi_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_semfi_cfg_update(wtk_semfi_cfg_t *cfg,wtk_lexc_t *lex);
int wtk_semfi_cfg_update2(wtk_semfi_cfg_t *cfg,wtk_source_loader_t *sl,wtk_lexc_t *lex);
#ifdef __cplusplus
};
#endif
#endif
