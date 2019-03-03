#ifndef WTK_HTTP_NBC_WTK_NBC_CFG_H_
#define WTK_HTTP_NBC_WTK_NBC_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/http/nk/wtk_nk_cfg.h"
#include "wtk/http/misc/httpnc/wtk_httpnc.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nbc_cfg wtk_nbc_cfg_t;

struct wtk_nbc_cfg
{
	wtk_nk_cfg_t nk;
	//wtk_httpnc_cfg_t httpnc;
};

int wtk_nbc_cfg_init(wtk_nbc_cfg_t *cfg);
int wtk_nbc_cfg_clean(wtk_nbc_cfg_t *cfg);
int wtk_nbc_cfg_update_local(wtk_nbc_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_nbc_cfg_update(wtk_nbc_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
