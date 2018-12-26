#ifndef WTK_DNNSVR_WTK_DNNSVRD
#define WTK_DNNSVR_WTK_DNNSVRD
#include "wtk/core/wtk_type.h" 
#include "wtk_dnnsvrd_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_dnnsvrd wtk_dnnsvrd_t;

struct wtk_dnnsvrd
{
	wtk_dnnsvrd_cfg_t *cfg;
	wtk_daemon_t *daemon;
	wtk_dnnsvr_t *dnnsvr;
};

wtk_dnnsvrd_t* wtk_dnnsvrd_new(wtk_dnnsvrd_cfg_t *cfg);
void wtk_dnnsvrd_delete(wtk_dnnsvrd_t *s);
int wtk_dnnsvrd_run(wtk_dnnsvrd_t *s);
#ifdef __cplusplus
};
#endif
#endif
