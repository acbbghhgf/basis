#include "wtk_crfact_parser_cfg.h" 

int wtk_crfact_parser_cfg_init(wtk_crfact_parser_cfg_t *cfg)
{
	wtk_poseg_cfg_init(&(cfg->poseg));
	cfg->model=NULL;
	return 0;
}

int wtk_crfact_parser_cfg_clean(wtk_crfact_parser_cfg_t *cfg)
{
	wtk_poseg_cfg_clean(&(cfg->poseg));
	return 0;
}

int wtk_crfact_parser_cfg_update_local(wtk_crfact_parser_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;

	lc=main;
	wtk_local_cfg_update_cfg_str(lc,cfg,model,v);
	lc=wtk_local_cfg_find_lc_s(main,"poseg");
	if(lc)
	{
		wtk_poseg_cfg_update_local(&(cfg->poseg),lc);
	}
	return 0;
}

int wtk_crfact_parser_cfg_update(wtk_crfact_parser_cfg_t *cfg)
{
	int ret;

	ret=wtk_poseg_cfg_update(&(cfg->poseg));
	if(ret!=0){goto end;}
end:
	return ret;
}
