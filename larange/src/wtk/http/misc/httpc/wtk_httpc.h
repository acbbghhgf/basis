#ifndef WTK_EXT_HTTPC_WTK_HTTPC_H_
#define WTK_EXT_HTTPC_WTK_HTTPC_H_
#include "wtk_httpc_cfg.h"
#include "wtk_httpc_request.h"
#include "wtk/http/misc/wtk_http_util.h"
#include "wtk/os/wtk_log.h"
#ifdef __cplusplus
extern "C" {
#endif
#define wtk_httpc_req_s(httpc,p) wtk_httpc_req(httpc,p,sizeof(p)-1)
typedef struct wtk_httpc wtk_httpc_t;

struct wtk_httpc
{
	wtk_httpc_cfg_t *cfg;
	wtk_httpc_request_t *req;
	wtk_strbuf_t *buf;
};

wtk_httpc_t* wtk_httpc_new(wtk_httpc_cfg_t *cfg);
int wtk_httpc_delete(wtk_httpc_t *httpc);
int wtk_httpc_reset(wtk_httpc_t *httpc);
int wtk_httpc_req(wtk_httpc_t* httpc,char *param,int param_bytes);
int wtk_httpc_post(wtk_httpc_t* httpc,char *param,int param_bytes);
void wtk_httpc_set_hdr_pad(wtk_httpc_t *httpc,void *add_hdr_ths,wtk_httpc_request_add_hdr_f add_hdr);
wtk_string_t wtk_httpc_get(wtk_httpc_t *httpc);
//-------------------- test/example section ---------------
void wtk_httpc_test_g();

int wtk_httpc_wget(char *url,void *ths,wtk_httpc_response_notify_f notify);
int wtk_httpc_wget2(char *url,void *ths,wtk_httpc_response_notify_f notify,int timeout);
int wtk_httpc_wget3(char *url,void *ths,wtk_httpc_response_notify_f notify,int timeout,int use_hack);
int wtk_httpc_wget4(char *url,void *ths,wtk_httpc_response_notify_f notify,int timeout,int use_hack,int *run);
#ifdef __cplusplus
};
#endif
#endif
