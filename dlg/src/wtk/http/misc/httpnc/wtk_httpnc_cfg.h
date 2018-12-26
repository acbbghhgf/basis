#ifndef WTK_HTTP_MISC_HTTPNC_WTK_HTTPNC_CFG_H_
#define WTK_HTTP_MISC_HTTPNC_WTK_HTTPNC_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/os/wtk_socket.h"
#include "wtk/http/misc/cookie/wtk_cookie_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_httpnc_cfg wtk_httpnc_cfg_t;
struct wtk_httpnc_cfg
{
	wtk_cookie_cfg_t cookie;
	wtk_string_t ip;
	wtk_string_t port;
	wtk_addrinfo_t *addr;
	int time_relink;
	unsigned use_1_1:1;
};

int wtk_httpnc_cfg_init(wtk_httpnc_cfg_t *cfg);
int wtk_httpnc_cfg_clean(wtk_httpnc_cfg_t *cfg);
int wtk_httpnc_cfg_update_local(wtk_httpnc_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_httpnc_cfg_update(wtk_httpnc_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
