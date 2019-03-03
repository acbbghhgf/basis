#ifndef WTK_EXT_HTTPC_WTK_HTTPC_REQUEST_H_
#define WTK_EXT_HTTPC_WTK_HTTPC_REQUEST_H_
#include "wtk/core/wtk_strbuf.h"
#include "wtk/os/wtk_fd.h"
#include "wtk/http/misc/wtk_http_response.h"
#include "wtk_httpc_cfg.h"
#include "wtk/os/wtk_log.h"
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
#define wtk_httpc_request_req_s(req,p) wtk_httpc_request_req(req,p,sizeof(p)-1)
typedef struct wtk_httpc_request wtk_httpc_request_t;
typedef void(*wtk_httpc_request_add_hdr_f)(void *ths,wtk_strbuf_t *buf);

typedef void(*wtk_httpc_response_notify_f)(void *ths,char *data,int len);

struct wtk_httpc_request
{
	wtk_httpc_cfg_t *cfg;
	wtk_string_t *url;
	wtk_log_t *log;
	char *ip;
	char *port;
	wtk_http_response_t* response;
	wtk_strbuf_t *buf;
	int fd;
	wtk_httpc_request_add_hdr_f add_hdr_f;
	void *add_hdr_ths;
	wtk_httpc_response_notify_f body_notify_f;
	void *body_notify_ths;
};

wtk_httpc_request_t* wtk_httpc_request_new(wtk_httpc_cfg_t *cfg);
wtk_httpc_request_t* wtk_httpc_request_new_fd(int fd,wtk_string_t *url);
void wtk_httpc_request_set_notify(wtk_httpc_request_t *req,void *ths,wtk_httpc_response_notify_f notify);
wtk_httpc_request_t* wtk_httpc_request_new_addr(wtk_string_t *url,struct sockaddr* addr,socklen_t len);
int wtk_httpc_request_delete(wtk_httpc_request_t *req);
void wtk_httpc_request_close(wtk_httpc_request_t *req);
int wtk_httpc_request_req(wtk_httpc_request_t *req,char *param,int param_bytes,int is_post,int use_1_1);
int wtk_httpc_request_req2(wtk_httpc_request_t *req,char *param,int param_bytes,int is_post,int use_1_1,int *run);
void wtk_httpc_request_set_client(wtk_string_t *v);
int wtk_httpc_request_reconnect(wtk_httpc_request_t *req);
int wtk_httpc_request_connect(char *ip,char *port,int *pfd,int timeout);

#ifdef __cplusplus
};
#endif
#endif
