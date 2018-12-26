#include "wtk_ps_cfg.h"
#ifdef WIN32
#else
#include <sys/stat.h>
#endif

int wtk_ps_cfg_init(wtk_ps_cfg_t *cfg)
{
	cfg->buf_size=1024;
	cfg->timeout=5000;
	cfg->select_timeout=100;
	cfg->cmd_fn=NULL;
	return 0;
}

int wtk_ps_cfg_clean(wtk_ps_cfg_t *cfg)
{
	return 0;
}

int wtk_ps_cfg_update_local(wtk_ps_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_i(lc,cfg,buf_size,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,timeout,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,select_timeout,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,cmd_fn,v);
	return 0;
}

int wtk_ps_cfg_update(wtk_ps_cfg_t *cfg)
{
	if(cfg->cmd_fn)
	{
#ifdef WIN32
#else
		chmod(cfg->cmd_fn,S_IRWXU);
#endif
	}
	return 0;
}
