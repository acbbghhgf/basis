#ifndef WTK_API_SPEECHC_WTK_OGGENC_CFG
#define WTK_API_SPEECHC_WTK_OGGENC_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "ogg.h"
#include "speex.h"
#include "speex_header.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_oggenc_cfg wtk_oggenc_cfg_t;
struct wtk_oggenc_cfg
{
    int spx_quality;
    int spx_complexity;
    int spx_vbr;
};

int wtk_oggenc_cfg_init(wtk_oggenc_cfg_t *cfg);
int wtk_oggenc_cfg_clean(wtk_oggenc_cfg_t *cfg);
int wtk_oggenc_cfg_update_local(wtk_oggenc_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_oggenc_cfg_update(wtk_oggenc_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
