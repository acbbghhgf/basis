#ifndef WTK_HTTP_LOC_REDIRECT_POOL_WTK_RELAY_POOL_H_
#define WTK_HTTP_LOC_REDIRECT_POOL_WTK_RELAY_POOL_H_
#include "wtk/core/wtk_type.h"
#include "wtk/os/wtk_lock.h"
#include "wtk_relay_pool_cfg.h"
#include "wtk/http/proto/wtk_request.h"
#include "wtk/http/misc/wtk_http_response.h"
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
typedef struct wtk_relay_pool wtk_relay_pool_t;
struct wtk_relay_pool
{
	wtk_relay_pool_cfg_t *cfg;
	wtk_lock_t lock;
	int fd;
	wtk_http_response_t *response;
	wtk_strbuf_t *buf;
};

wtk_relay_pool_t* wtk_relay_pool_new(wtk_relay_pool_cfg_t *cfg);
void wtk_relay_pool_delete(wtk_relay_pool_t *p);

int wtk_relay_pool_process(wtk_relay_pool_t *p,wtk_request_t *req);
#ifdef __cplusplus
};
#endif
#endif
