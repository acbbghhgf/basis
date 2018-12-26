#ifndef WTK_HTTP_MISC_HTTPNC_WTK_HTTPNC_H_
#define WTK_HTTP_MISC_HTTPNC_WTK_HTTPNC_H_
#include "wtk/core/wtk_type.h"
#include "wtk/http/misc/wtk_http_response.h"
#include "wtk/http/nk/wtk_nk.h"
#include "wtk/os/wtk_sem.h"
#include "wtk_httpnc_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_httpnc wtk_httpnc_t;
typedef enum
{
	WTK_HTTPNC_RESPONSE,
	WTK_HTTPNC_DISCONNECT,
}wtk_httpnc_notify_type_t;

typedef void(*wtk_httpnc_add_hdr_f)(void *ths,wtk_strbuf_t *buf);
typedef void(*wtk_httpnc_notify_f)(void *ths,wtk_httpnc_notify_type_t type,wtk_http_response_t *response);
struct wtk_nbc;

typedef struct
{
	wtk_queue_node_t pipe_n;			//used for nk pipe;
	wtk_httpnc_t *httpnc;
	void *ths;
	wtk_httpnc_add_hdr_f add_hdr;
	wtk_httpnc_notify_f notify;
	wtk_string_t url;
	wtk_string_t param;
	unsigned use_get:1;
}wtk_httpnc_req_t;


struct wtk_httpnc
{
	WTK_PARSER
	wtk_timer_t connect_timer;		//used for connect when disconnect;
	wtk_queue_node_t pend_n;		//used for node in nk;
	wtk_httpnc_cfg_t *cfg;
	struct wtk_nbc *nbc;
	wtk_http_response_t *response;
	wtk_connection_t *con;
	wtk_httpnc_req_t *req;
};

wtk_httpnc_t* wtk_httpnc_new(wtk_httpnc_cfg_t *cfg,struct wtk_nbc *nbc);
void wtk_httpnc_delete(wtk_httpnc_t *h);
int wtk_httpnc_connect(wtk_httpnc_t *h);
void wtk_httpnc_request(wtk_httpnc_t *h,wtk_httpnc_req_t *req);
#ifdef __cplusplus
};
#endif
#endif
