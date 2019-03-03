#ifndef WTK_VM_CACHE_WTK_CACHE_CFG_H_
#define WTK_VM_CACHE_WTK_CACHE_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/http/misc/httpc/wtk_httpc.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_cache_cfg wtk_cache_cfg_t;
struct wtk_cache_cfg
{
	char db_buf_fn[64];
	char port_mem[16];
	wtk_httpc_cfg_t httpc;
	wtk_string_t db_fn;
	wtk_string_t http_port;
	int nslot;
	int max_active_slot;
	int httpc_retry;
	unsigned use_httpc:1;
	unsigned use_db:1;
};

int wtk_cache_cfg_init(wtk_cache_cfg_t *cfg);
int wtk_cache_cfg_clean(wtk_cache_cfg_t *cfg);
int wtk_cache_cfg_update(wtk_cache_cfg_t *cfg);
int wtk_cache_cfg_update_local(wtk_cache_cfg_t *cfg,wtk_local_cfg_t *lc);
#ifdef __cplusplus
};
#endif
#endif
