#ifndef WTK_HTTP_NBC_WTK_NBC_H_
#define WTK_HTTP_NBC_WTK_NBC_H_
#include "wtk/http/nk/wtk_nk.h"
#include "wtk_nbc_cfg.h"
#include "wtk/os/wtk_thread.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nbc wtk_nbc_t;

struct wtk_nbc
{
	wtk_nbc_cfg_t *cfg;
	wtk_nk_t *nk;
	wtk_queue_t httpnc_q;	//wtk_httpnc pend q;
	wtk_thread_t thread;
};

wtk_nbc_t* wtk_nbc_new(wtk_nbc_cfg_t *cfg,wtk_log_t *log);
void wtk_nbc_delete(wtk_nbc_t *nbc);
void wtk_nbc_add_httpnc(wtk_nbc_t *nbc,wtk_httpnc_t *httpnc);
void wtk_nbc_send_request(wtk_nbc_t *nbc,wtk_httpnc_req_t *req);
int wtk_nbc_start(wtk_nbc_t *nbc);
int wtk_nbc_stop(wtk_nbc_t *nbc);
#ifdef __cplusplus
};
#endif
#endif
