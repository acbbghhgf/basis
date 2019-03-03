#ifndef WTK_HTTP2_WTK_HTTP2_H_
#define WTK_HTTP2_WTK_HTTP2_H_
#include "wtk/http/wtk_http.h"
#include "wtk/os/wtk_thread.h"
#include "wtk/http/http2/ctx/wtk_http2_ctx.h"
#include "wtk_http2_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_http2 wtk_http2_t;

struct wtk_http2
{
	wtk_http2_cfg_t *cfg;
	wtk_log_t *log;
	wtk_http_t *https;
	wtk_http2_ctx_t *ctx;
	wtk_thread_t *threads;
	wtk_listen_t listen;
	wtk_strbuf_t *tmp_buf;
};

wtk_http2_t* wtk_http2_new(wtk_http2_cfg_t *cfg,wtk_log_t *log,
		void *nke_hook,wtk_http_nke_handler_t nke_handler);
void wtk_http2_delete(wtk_http2_t *h);

/**
 * @brief start http server route;
 */
int wtk_http2_start(wtk_http2_t *h);

/**
 * @brief stop http server route;
 */
int wtk_http2_stop(wtk_http2_t *h);

/**
 * @brief join http server route;
 */
int wtk_http2_join(wtk_http2_t *h);

/**
 * @brief check request raise able or not;
 */
void wtk_http2_set_raise_able_f(wtk_http2_t *h,void *ths,wtk_http_reuqest_raise_able_f raise);


/**
 * @brief set response filter;
 */
void wtk_http2_set_response_filter(wtk_http2_t *h,void *hook,wtk_http_response_filter_f filter);
//------------------- private section ---------------------
void wtk_http2_get_statics(wtk_http2_t* h,wtk_strbuf_t *buf);
int wtk_http2_get_conection_count(wtk_http2_t *h);
#ifdef __cplusplus
};
#endif
#endif
