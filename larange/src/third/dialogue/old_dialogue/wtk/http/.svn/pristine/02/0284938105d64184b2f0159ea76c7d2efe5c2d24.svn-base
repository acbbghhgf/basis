#ifndef WTK_EXT_HTTPC_WTK_HTTPC_CFG_H_
#define WTK_EXT_HTTPC_WTK_HTTPC_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/http/misc/cookie/wtk_cookie_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_httpc_cfg wtk_httpc_cfg_t;

struct wtk_httpc_cfg
{
	char ip_buf[64];
	char port_buf[16];
	wtk_cookie_cfg_t cookie;
	wtk_string_t url;
	wtk_string_t ip;
	wtk_string_t port;
	int timeout;	//ms
	unsigned use_1_1:1;
	unsigned use_hack:1;
};

wtk_httpc_cfg_t* wtk_httpc_cfg_new();
int wtk_httpc_cfg_delete(wtk_httpc_cfg_t *cfg);
int wtk_httpc_cfg_init(wtk_httpc_cfg_t *cfg);
int wtk_httpc_cfg_clean(wtk_httpc_cfg_t *cfg);
int wtk_httpc_cfg_update(wtk_httpc_cfg_t *cfg);
int wtk_httpc_cfg_update_local(wtk_httpc_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_httpc_cfg_update_srv(wtk_httpc_cfg_t *cfg,wtk_string_t *host,wtk_string_t *port,int timeout);
#ifdef __cplusplus
};
#endif
#endif
