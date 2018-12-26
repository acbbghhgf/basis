#include "wtk_lua2_cfg.h"

int wtk_lua2_cfg_init(wtk_lua2_cfg_t *cfg)
{
	//cfg->post_fn=0;
	cfg->libs=0;
	cfg->fn=0;
	cfg->local_lang="zh_CN.UTF-8";
	cfg->include_path=0;
	return 0;
}

int wtk_lua2_cfg_clean(wtk_lua2_cfg_t *cfg)
{
	return 0;
}

int wtk_lua2_cfg_update_local(wtk_lua2_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	//wtk_local_cfg_print(lc);
	cfg->libs=wtk_local_cfg_find_array_s(lc,"libs");
	cfg->include_path=wtk_local_cfg_find_array_s(lc,"include_path");
	//wtk_debug("libs=%p\n",cfg->libs);
	wtk_local_cfg_update_cfg_str(lc,cfg,fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,local_lang,v);
	return 0;
}

int wtk_lua2_cfg_update(wtk_lua2_cfg_t *cfg)
{
	return 0;
}
