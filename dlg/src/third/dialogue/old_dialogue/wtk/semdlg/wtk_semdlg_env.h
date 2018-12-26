#ifndef WTK_SEMDLG_WTK_SEMDLG_ENV
#define WTK_SEMDLG_WTK_SEMDLG_ENV
#include "wtk/core/wtk_type.h" 
#include "wtk/core/cfg/wtk_local_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_semdlg_env wtk_semdlg_env_t;

struct wtk_semdlg_env
{
	wtk_string_t client;
};

void wtk_semdlg_env_init(wtk_semdlg_env_t *env);
void wtk_semdlg_env_clean(wtk_semdlg_env_t *env);
void wtk_semdlg_env_reset(wtk_semdlg_env_t *env);
void wtk_semdlg_env_update_local(wtk_semdlg_env_t *env,wtk_local_cfg_t *lc);
#ifdef __cplusplus
};
#endif
#endif
