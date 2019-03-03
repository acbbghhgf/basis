#ifndef WTK_CORE_CFG_WTK_VERSION_CFG_H_
#define WTK_CORE_CFG_WTK_VERSION_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_version_cfg wtk_version_cfg_t;
struct wtk_version_cfg
{
	char version_buf[sizeof("000.000.000.2012.00.00.00:00:00")];
	wtk_string_t ver;
};

int wtk_version_cfg_init(wtk_version_cfg_t *cfg,char *v);
int wtk_version_cfg_clean(wtk_version_cfg_t *cfg);
int wtk_version_cfg_update_local(wtk_version_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_version_cfg_update(wtk_version_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
