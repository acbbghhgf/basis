#ifndef WTK_HTTP_LOC_WTK_LOC_CFG_H_
#define WTK_HTTP_LOC_WTK_LOC_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/http/loc/lfs/wtk_lfs_cfg.h"
#include "wtk/http/loc/cd/wtk_cd_cfg.h"
#ifdef WIN32
#else
#include "wtk/http/loc/redirect/wtk_redirect.h"
#endif
#ifdef USE_STATICS
#include "wtk/http/loc/statics/wtk_statics_cfg.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_loc_cfg wtk_loc_cfg_t;

struct wtk_loc_cfg
{
	wtk_cd_cfg_t cd;
	wtk_lfs_cfg_t lfs;
#ifdef WIN32
#else
	wtk_redirect_cfg_t redirect;
#endif
#ifdef USE_STATICS
	wtk_statics_cfg_t statics;
#endif
	wtk_string_t url_root;
	int hash_nslot;
	unsigned use_lfs:1;
	unsigned use_root:1;
	unsigned use_statics:1;
	unsigned show_root:1;
};

int wtk_loc_cfg_init(wtk_loc_cfg_t *cfg);
int wtk_loc_cfg_clean(wtk_loc_cfg_t *cfg);
int wtk_loc_cfg_update_local(wtk_loc_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_loc_cfg_update(wtk_loc_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
