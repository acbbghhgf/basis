#ifndef WTK_HTTP_LOC_STATICS_WTK_STATICS_CFG_H_
#define WTK_HTTP_LOC_STATICS_WTK_STATICS_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_statics_cfg wtk_statics_cfg_t;
struct wtk_statics_cfg
{
	wtk_string_t url_statics;
	wtk_string_t url_debug;
	wtk_string_t url_speech;
	wtk_string_t url_cpu;
};

int wtk_statics_cfg_init(wtk_statics_cfg_t *cfg);
int wtk_statics_cfg_clean(wtk_statics_cfg_t *cfg);
int wtk_statics_cfg_update_local(wtk_statics_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_statics_cfg_update(wtk_statics_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
