#ifndef WTK_HTTP_LOC_LFS_WTK_LFS_H_
#define WTK_HTTP_LOC_LFS_WTK_LFS_H_
#include "wtk/core/wtk_type.h"
#include "wtk/http/proto/wtk_request.h"
#include "wtk/http/proto/wtk_response.h"
#include "wtk_lfs_cfg.h"
#ifdef WIN32
#include <sys/stat.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lfs wtk_lfs_t;

struct wtk_lfs
{
	wtk_lfs_cfg_t *cfg;
	wtk_strbuf_t *buf;
	wtk_strbuf_t *tmp_buf;
	wtk_param_t param;
};

wtk_lfs_t* wtk_lfs_new(wtk_lfs_cfg_t *cfg);
void wtk_lfs_delete(wtk_lfs_t *l);
int wtk_lfs_process(wtk_lfs_t *l,wtk_request_t *req);
#ifdef __cplusplus
};
#endif
#endif
