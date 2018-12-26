#include "wtk_semfstr_cfg.h" 

int wtk_semfstr_cfg_init(wtk_semfstr_cfg_t *cfg)
{
	cfg->dn="./semfstr";
	cfg->lua_save="wtk_semfstr_save";
	cfg->lua_ask="wtk_semfstr_ask";
	cfg->lua_main="wtk_semfstr_main";
	cfg->lua=NULL;
	return 0;
}

int wtk_semfstr_cfg_clean(wtk_semfstr_cfg_t *cfg)
{
	return 0;
}

int wtk_semfstr_cfg_update_local(wtk_semfstr_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;

	lc=main;
	wtk_local_cfg_update_cfg_str(lc,cfg,dn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_save,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_ask,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_main,v);
	cfg->lua=wtk_local_cfg_find_array_s(lc,"lua");
	return 0;
}

int wtk_semfstr_cfg_update(wtk_semfstr_cfg_t *cfg)
{
	return 0;
}
