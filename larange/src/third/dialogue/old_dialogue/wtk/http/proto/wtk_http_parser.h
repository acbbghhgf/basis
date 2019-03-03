#ifndef WTK_NK_WTK_HTTP_PARSER_H_
#define WTK_NK_WTK_HTTP_PARSER_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk/http/nk/listen/wtk_listen.h"
#include "wtk/http/nk/wtk_connection.h"
#include "wtk_request.h"
#include "wtk_response.h"
#ifdef __cplusplus
extern "C" {
#endif
struct wtk_http;
typedef struct wtk_http_parser wtk_http_parser_t;

struct wtk_http_parser
{
	WTK_PARSER
	wtk_queue_node_t n;
	void *layer;
	wtk_request_t *request;
	wtk_connection_t *con;
	wtk_queue_t stream_queue;
	wtk_nk_event_t	nk_to_vm_event;
};
wtk_http_parser_t* wtk_http_parser_new(struct wtk_http *http);
int wtk_http_parser_delete(wtk_http_parser_t *p);
int wtk_http_parser_bytes(wtk_http_parser_t *p);
int wtk_http_parser_init(wtk_http_parser_t *p,wtk_connection_t* c);
int wtk_http_parser_reset(wtk_http_parser_t *p);
int wtk_http_parser_feed_hdr(wtk_http_parser_t *p,wtk_connection_t *c,char *buf,int len);
int wtk_http_parser_feed_content(wtk_http_parser_t *p,wtk_connection_t *c,char *buf,int len);
int wtk_http_parser_done_request(wtk_http_parser_t *p,wtk_request_t *req);
void wtk_http_parser_try_raise(wtk_http_parser_t *p,wtk_connection_t *c);
int wtk_http_parser_notify_close(wtk_http_parser_t *p);
int wtk_http_parser_done_unlink(wtk_http_parser_t *p);
int wtk_http_parser_raise_request(wtk_http_parser_t *p,wtk_request_t *req);
int wtk_http_parser_push_stream(wtk_http_parser_t *p,wtk_queue_node_t *n);
int wtk_http_parser_remove_stream(wtk_http_parser_t *p,wtk_queue_node_t *n);
wtk_queue_node_t* wtk_http_parser_pop_stream(wtk_http_parser_t *p);
void wtk_http_parser_hook_request(wtk_http_parser_t *p,wtk_request_t *req);
#ifdef __cplusplus
};
#endif
#endif
