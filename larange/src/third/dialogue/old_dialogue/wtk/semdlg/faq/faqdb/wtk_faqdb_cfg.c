#include "wtk_faqdb_cfg.h" 

int wtk_faqdb_cfg_init(wtk_faqdb_cfg_t *cfg)
{
	cfg->db=NULL;
	return 0;
}

int wtk_faqdb_cfg_clean(wtk_faqdb_cfg_t *cfg)
{
	return 0;
}

int wtk_faqdb_cfg_update_local(wtk_faqdb_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;
	int ret;

	lc=main;
	wtk_local_cfg_update_cfg_str(lc,cfg,db,v);
	ret=0;
	return ret;
}


int wtk_faqdb_cfg_update(wtk_faqdb_cfg_t *cfg)
{
	return 0;
}
