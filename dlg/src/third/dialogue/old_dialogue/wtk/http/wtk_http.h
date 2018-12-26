#ifndef WTK_NK_WTK_HTTP_H_
#define WTK_NK_WTK_HTTP_H_
#include "wtk/core/wtk_type.h"
#include "wtk/http/proto/wtk_http_parser.h"
#include "wtk/http/nk/wtk_nk.h"
#include "wtk/os/wtk_pipequeue.h"
#include "wtk/os/wtk_lockqueue.h"
#include "wtk/os/wtk_thread.h"
#include "wtk/http/loc/wtk_loc.h"
#include "wtk/http/pp/wtk_pp.h"
#include "wtk/http/plink/wtk_plink.h"
#include "wtk/http/wtk_http_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_http wtk_http_t;
typedef int (*wtk_http_nke_handler_t)(void *data,wtk_nk_event_t *nke);
typedef int (*wtk_http_reuqest_raise_able_f)(void *data,wtk_request_t *req);
typedef void (*wtk_http_response_filter_f)(void *hook,wtk_request_t *req);

struct wtk_http
{
	wtk_http_cfg_t *cfg;
	wtk_nk_t *net;
	wtk_heap_t  *heap;
	wtk_hoard_t *parser_hoard;
	wtk_hoard_t *request_hoard;
    wtk_loc_t *loc;
//=================== proxy server ==================
	wtk_plink_t *plink;
	wtk_pp_t *pp;
	wtk_http_reuqest_raise_able_f request_raise_able;
	void *request_raise_able_hook;
//=================== used for single layer process ==============
	wtk_http_nke_handler_t nke_handler;
	void *nke_hook;
//=================== used for filter response =====================
	wtk_http_response_filter_f response_filter;
	void *response_filter_hook;
};

int wtk_http_init(wtk_http_t* http,wtk_http_cfg_t *cfg,wtk_log_t *log,void *nke_hook,wtk_http_nke_handler_t nke_handler);
int wtk_http_bytes(wtk_http_t *http);
int wtk_http_clean(wtk_http_t *http);
int wtk_http_run(wtk_http_t *http,wtk_thread_t *t);
int wtk_http_prepare(wtk_http_t *http);

/**
 *	@brief use listened fd initialize;
 *	@param listened already listen fd;
 */
int wtk_http_prepare2(wtk_http_t *http,wtk_listen_t *listened);

/**
 * @brief send stop hint to http;
 */
int wtk_http_stop(wtk_http_t *http);

/**
 * @brief check request can be raised or not;
 */
void wtk_http_request_set_raise_able(wtk_http_t *http,void *hook,wtk_http_reuqest_raise_able_f raise);

/**
 * @brief set response write callback;
 */
void wtk_http_set_response_filter(wtk_http_t *http,void *hook,wtk_http_response_filter_f filter);

//------------------------ private section ------------------------
int wtk_http_start_listen(wtk_http_t *http,wtk_listen_t *lfd,int accept_sem);
wtk_request_t* wtk_http_new_request(wtk_http_t *http);
wtk_request_t* wtk_http_pop_request(wtk_http_t *http,wtk_connection_t *c);
int wtk_http_push_request(wtk_http_t *http,wtk_request_t* r);
wtk_http_parser_t* wtk_http_new_parser(wtk_http_t *http);
wtk_http_parser_t* wtk_http_pop_parser(wtk_http_t *http,wtk_connection_t *c);
int wtk_http_push_parser(wtk_http_t *http,wtk_http_parser_t* p);
int wtk_http_read_vm(wtk_http_t* http,wtk_event_t *e);
int wtk_http_do_location(wtk_http_t *http,wtk_request_t *req);
int wtk_http_close_fd(wtk_http_t *http);
int wtk_http_raise_event(wtk_http_t *http,wtk_nk_event_t *event);
int wtk_http_lockqueue_nke_handler(wtk_lockqueue_t* lk,wtk_nk_event_t *nke);
void wtk_http_clean_mem(wtk_http_t *http);

int wtk_http_request_is_raise_able(wtk_http_t *http,wtk_request_t *req);
int wtk_nk_event_done(wtk_nk_event_t *nke);
#ifdef __cplusplus
};
#endif
#endif
