#ifndef WTK_HTTP_LOC_REDIRECT_HOST_WTK_RELAY_HOST_H_
#define WTK_HTTP_LOC_REDIRECT_HOST_WTK_RELAY_HOST_H_
#include "wtk/core/wtk_type.h"
#include "wtk/http/proto/wtk_request.h"
#include "wtk/http/misc/wtk_http_response.h"
#include "wtk_relay_host_cfg.h"
#ifdef WIN32
#include "wtk/os/wtk_socket.h"
#else
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_relay_host wtk_relay_host_t;

struct wtk_relay_host
{
	wtk_relay_host_cfg_t *cfg;
	wtk_strbuf_t *tmp_buf;
	int fd;
	wtk_http_response_t* response;
};

wtk_relay_host_t* wtk_relay_host_new(wtk_relay_host_cfg_t *cfg,wtk_strbuf_t *tmp_buf);
void wtk_relay_host_delete(wtk_relay_host_t *host);
int wtk_relay_host_process(wtk_relay_host_t *host,wtk_request_t *req);
#ifdef __cplusplus
};
#endif
#endif
