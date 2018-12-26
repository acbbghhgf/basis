#include "wtk_owlkg_cfg.h" 

int wtk_owlkg_cfg_init(wtk_owlkg_cfg_t *cfg)
{
	cfg->lua=NULL;
	cfg->nlg=NULL;
	cfg->owl=NULL;
	cfg->brain_dn=NULL;
	return 0;
}

int wtk_owlkg_cfg_clean(wtk_owlkg_cfg_t *cfg)
{
	return 0;
}

int wtk_owlkg_cfg_update_local(wtk_owlkg_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_str(lc,cfg,brain_dn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,nlg,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,owl,v);
	cfg->lua=wtk_local_cfg_find_array_s(lc,"lua");
	return 0;
}

int wtk_owlkg_cfg_update(wtk_owlkg_cfg_t *cfg)
{
	return 0;
}
