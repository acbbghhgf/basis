#ifndef WTK_PROXY_LOC_WTK_LOC_TREE_H_
#define WTK_PROXY_LOC_WTK_LOC_TREE_H_
#include "wtk/core/wtk_str_hash.h"
#include "wtk/http/proto/wtk_request.h"
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/core/param/wtk_param.h"
#include "wtk/http/proto/wtk_response.h"
#include "wtk/http/loc/lfs/wtk_lfs.h"
#ifdef WIN32
#else
#include "wtk/http/loc/redirect/wtk_redirect.h"
#endif
#include "wtk_loc_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_loc wtk_loc_t;
typedef int (*wtk_loc_ctx_process_f)(void *data,wtk_request_t *req);
struct wtk_http;

typedef struct
{
	wtk_queue_node_t q_n;
	hash_str_node_t hash_n;
	wtk_loc_ctx_process_f process;
	void *data;
}wtk_loc_ctx_t;

struct wtk_loc
{
	wtk_loc_cfg_t *cfg;
	wtk_str_hash_t *ctx_hash;
	wtk_queue_t ctx_q;
	wtk_lfs_t *lfs;
#ifdef WIN32
#else
	wtk_redirect_t *redirect;
#endif
	wtk_loc_ctx_t *lost_ctx;
	struct wtk_http *http;
};

wtk_loc_t* wtk_loc_new(wtk_loc_cfg_t *cfg,struct wtk_http *http);
int wtk_loc_delete(wtk_loc_t *t);
wtk_loc_ctx_t* wtk_loc_add_ctx(wtk_loc_t *l,wtk_string_t *url,void *data,wtk_loc_ctx_process_f process);
int wtk_loc_do(wtk_loc_t *t,wtk_request_t *req);
#ifdef __cplusplus
};
#endif
#endif
