#ifndef WTK_HTTP_WTK_HTTP_CFG_H_
#define WTK_HTTP_WTK_HTTP_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/http/nk/wtk_nk_cfg.h"
#include "wtk/http/loc/wtk_loc_cfg.h"
#include "wtk/http/pp/wtk_pp_cfg.h"
#include "wtk/http/proto/wtk_request_cfg.h"
#include "wtk/http/plink/wtk_plink_cfg.h"
#include "wtk/core/wtk_arg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_http_cfg wtk_http_cfg_t;
extern char* http_status_map[520];

#define WTK_HTTP_PROTO_ERR 400
#define WTK_HTTP_LOGIC_ERR 500
#define WTK_HTTP_BUSY 503

struct wtk_http_cfg
{
	wtk_loc_cfg_t loc;
	wtk_nk_cfg_t nk;
	wtk_pp_cfg_t pp;
	wtk_plink_cfg_t plink;
	wtk_request_cfg_t request;
	wtk_response_cfg_t response;
	wtk_string_t iface;
	wtk_strbuf_t *http_lfs_url;
    char ver_buf[sizeof("00.00.00.2012.00.10.21:48:41:000000000")];
    wtk_string_t ver;
    wtk_string_t server;
	int parser_cache;
    int port;
	int cpus;
    int max_streams;
    int max_streams_per_connection;
    float stream_per_cpu;
    float stream_per_hz;
    unsigned streams_by_cpu:1;
    unsigned streams_use_frequency:1;
    //unsigned use_pipe:1;
    unsigned use_pp:1;
    unsigned use_delay_hint:1;
    unsigned log_req_route:1;
};

int wtk_http_cfg_init(wtk_http_cfg_t *cfg);
int wtk_http_cfg_clean(wtk_http_cfg_t *cfg);
void wtk_http_cfg_set_max_stream(wtk_http_cfg_t *cfg,int max_active_stream);
int wtk_http_cfg_update(wtk_http_cfg_t *cfg);
void wtk_http_cfg_update_arg(wtk_http_cfg_t *cfg,wtk_arg_t *arg);
int wtk_http_cfg_update_local(wtk_http_cfg_t *cfg,wtk_local_cfg_t *lc);
void wtk_http_cfg_print(wtk_http_cfg_t *cfg);
void wtk_http_cfg_set_port(wtk_http_cfg_t *cfg,short port);
void wtk_http_cfg_print_usage();
#ifdef __cplusplus
};
#endif
#endif
