#ifndef WTK_HTTP_PP_WTK_PP_H_
#define WTK_HTTP_PP_WTK_PP_H_
#include "wtk/http/nk/wtk_nk.h"
#include "wtk/http/misc/wtk_http_response.h"
#include "wtk_pp_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_pp wtk_pp_t;
struct wtk_http;

struct wtk_pp
{
	WTK_PARSER
	wtk_timer_t timer;
	struct wtk_http* http;
	wtk_pp_cfg_t *cfg;
	wtk_http_response_t *response;
	wtk_connection_t *con;
	unsigned print_con_info:1;
};

wtk_pp_t* wtk_pp_new(wtk_pp_cfg_t *cfg,struct wtk_http* http);
int wtk_pp_delete(wtk_pp_t *p);
int wtk_pp_link(wtk_pp_t *p);
int wtk_pp_flush(wtk_pp_t *p);
#ifdef __cplusplus
};
#endif
#endif
