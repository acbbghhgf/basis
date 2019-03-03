#ifndef WTK_DNNSVR_WTK_DNNSVR
#define WTK_DNNSVR_WTK_DNNSVR
#include "wtk/core/wtk_type.h" 
#include "wtk/os/wtk_thread.h"
#include "wtk/nk/parser/wtk_msg_parser.h"
#include "wtk/os/daemon/wtk_daemon.h"
#include "wtk_dnnsvr_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_dnnsvr wtk_dnnsvr_t;

struct wtk_dnnsvr
{
	wtk_dnnsvr_cfg_t *cfg;
	wtk_dnnupsvr_t *upsvr;
	wtk_nk_t *nk;
	wtk_thread_t thread[2];
};

wtk_dnnsvr_t *wtk_dnnsvr_new(wtk_dnnsvr_cfg_t *cfg);
void wtk_dnnsvr_delete(wtk_dnnsvr_t *svr);
int wtk_dnnsvr_start(wtk_dnnsvr_t *svr);
int wtk_dnnsvr_stop(wtk_dnnsvr_t *svr);
int wtk_dnnsvr_join(wtk_dnnsvr_t *svr);
#ifdef __cplusplus
};
#endif
#endif
