#ifndef WTK_HTTP_LOC_LFS_WTK_LFS_CFG_H_
#define WTK_HTTP_LOC_LFS_WTK_LFS_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lfs_cfg wtk_lfs_cfg_t;
struct wtk_lfs_cfg
{
	wtk_heap_t *heap;
	wtk_string_t url;
	wtk_string_t dir;
	int buf_size;
	int buf_rate;
};

int wtk_lfs_cfg_init(wtk_lfs_cfg_t *cfg);
int wtk_lfs_cfg_clean(wtk_lfs_cfg_t *cfg);
int wtk_lfs_cfg_update_local(wtk_lfs_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_lfs_cfg_update(wtk_lfs_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
