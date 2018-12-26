#ifndef WTK_HTTP_PLINK_HOST_WTK_PLINK_HOST_H_
#define WTK_HTTP_PLINK_HOST_WTK_PLINK_HOST_H_
#include "wtk/http/misc/httpc/wtk_httpc.h"
#include "wtk/os/wtk_timer.h"
#include "wtk/http/proto/wtk_http_parser.h"
#include "wtk_plink_host_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
struct wtk_http;
typedef struct wtk_plink_host wtk_plink_host_t;
struct wtk_plink_host
{
	wtk_plink_host_cfg_t *cfg;
	wtk_httpc_t *httpc;
	struct wtk_http *http;
	wtk_timer_t timer;
	wtk_connection_listener_t listener;
};

wtk_plink_host_t* wtk_plink_host_new(wtk_plink_host_cfg_t *cfg,struct wtk_http *http);
void wtk_plink_host_delete(wtk_plink_host_t *host);
int wtk_plink_host_link(wtk_plink_host_t *host);
#ifdef __cplusplus
};
#endif
#endif
