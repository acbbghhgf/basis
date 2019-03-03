#include "wtk_version_cfg.h"
#include "wtk/core/wtk_os.h"


int wtk_version_cfg_init(wtk_version_cfg_t *cfg,char *v)
{
	char buf[sizeof("2012.00.00.00:00:00")];
	int n;

	memset(cfg,0,sizeof(*cfg));
	wtk_get_build_timestamp(buf);
	n=sprintf(cfg->version_buf,"%s.%s",v,buf);
	wtk_string_set(&(cfg->ver),cfg->version_buf,n);
	return 0;
}

int wtk_version_cfg_clean(wtk_version_cfg_t *cfg)
{
	return 0;
}

int wtk_version_cfg_update_local(wtk_version_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_string_v(lc,cfg,ver,v);
	return 0;
}

int wtk_version_cfg_update(wtk_version_cfg_t *cfg)
{
	return 0;
}
