#ifndef WTK_HTTP_LOC_CD_WTK_CD_CFG_H_
#define WTK_HTTP_LOC_CD_WTK_CD_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/http/proto/wtk_request.h"
#include "wtk/http/proto/wtk_response.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_cd_cfg wtk_cd_cfg_t;
struct wtk_cd_cfg
{
	wtk_string_t url;
};

int wtk_cd_cfg_init(wtk_cd_cfg_t *cfg);
int wtk_cd_cfg_clean(wtk_cd_cfg_t *cfg);
int wtk_cd_cfg_update_local(wtk_cd_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_cd_cfg_update(wtk_cd_cfg_t *cfg);

int wtk_cd_cfg_process(wtk_cd_cfg_t *cfg,wtk_request_t *req);
#ifdef __cplusplus
};
#endif
#endif
