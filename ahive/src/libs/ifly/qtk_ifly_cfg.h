#ifndef _QTK_SERVICE_LIBS_IFLY_QTK_IFLY_CFG_H
#define _QTK_SERVICE_LIBS_IFLY_QTK_IFLY_CFG_H
#include "wtk/core/cfg/wtk_local_cfg.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct qtk_ifly_cfg qtk_ifly_cfg_t;
struct qtk_ifly_cfg
{
	char *login_param;
	char *session_param;
};

int qtk_ifly_cfg_init(qtk_ifly_cfg_t *cfg);
int qtk_ifly_cfg_clean(qtk_ifly_cfg_t *cfg);
int qtk_ifly_cfg_update_local(qtk_ifly_cfg_t *cfg,wtk_local_cfg_t *lc);
int qtk_ifly_cfg_update(qtk_ifly_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
