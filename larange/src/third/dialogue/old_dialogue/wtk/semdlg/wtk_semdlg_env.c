#include "wtk_semdlg_env.h" 

void wtk_semdlg_env_init(wtk_semdlg_env_t *env)
{
	wtk_string_set(&(env->client),0,0);
}

void wtk_semdlg_env_clean(wtk_semdlg_env_t *env)
{
}

void wtk_semdlg_env_reset(wtk_semdlg_env_t *env)
{
	wtk_semdlg_env_clean(env);
	wtk_semdlg_env_init(env);
}

void wtk_semdlg_env_update_local(wtk_semdlg_env_t *env,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_string_v(lc,env,client,v);
}
