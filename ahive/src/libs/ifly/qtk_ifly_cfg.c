#include "qtk_ifly_cfg.h" 

int qtk_ifly_cfg_init(qtk_ifly_cfg_t *cfg)
{
	cfg->login_param="appid = 574c55b7, work_dir = .";
	cfg->session_param="sub = iat, domain = iat, language = zh_cn, accent = mandarin, sample_rate = 16000, result_type = plain, result_encoding = utf8,asr_ptt=0";
	//cfg->session_param="sub = iat, domain = iat, language = en_us, accent = mandarin, sample_rate = 16000, result_type = plain, result_encoding = utf8,asr_ptt=0";
	return 0;
}

int qtk_ifly_cfg_clean(qtk_ifly_cfg_t *cfg)
{
	return 0;
}

int qtk_ifly_cfg_update_local(qtk_ifly_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_str(lc,cfg,login_param,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,session_param,v);
	return 0;
}

int qtk_ifly_cfg_update(qtk_ifly_cfg_t *cfg)
{
	return 0;
}
