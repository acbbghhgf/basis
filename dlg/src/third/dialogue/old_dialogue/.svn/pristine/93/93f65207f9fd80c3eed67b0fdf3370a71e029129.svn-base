#ifndef WTK_SEMDLG_FAQ_WTK_FAQC_CFG
#define WTK_SEMDLG_FAQ_WTK_FAQC_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/http/misc/httpc/wtk_httpc.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_faqc_cfg wtk_faqc_cfg_t;
struct wtk_faqc_cfg
{
	wtk_httpc_cfg_t httpc;
	wtk_string_t appid;
	float conf;
};

int wtk_faqc_cfg_init(wtk_faqc_cfg_t *cfg);
int wtk_faqc_cfg_clean(wtk_faqc_cfg_t *cfg);
int wtk_faqc_cfg_update_local(wtk_faqc_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_faqc_cfg_update(wtk_faqc_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
