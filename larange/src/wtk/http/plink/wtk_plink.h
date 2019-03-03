#ifndef WTK_HTTP_PLINK_WTK_PLINK_H_
#define WTK_HTTP_PLINK_WTK_PLINK_H_
#include "wtk/http/plink/host/wtk_plink_host.h"
#include "wtk_plink_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_plink wtk_plink_t;
struct wtk_plink
{
	wtk_plink_cfg_t *cfg;
	wtk_plink_host_t **hosts;
};

wtk_plink_t* wtk_plink_new(wtk_plink_cfg_t *cfg,struct wtk_http *http);
void wtk_plink_delete(wtk_plink_t *p);
int wtk_plink_link(wtk_plink_t *p);
#ifdef __cplusplus
};
#endif
#endif
