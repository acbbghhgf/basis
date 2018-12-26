#include "wtk_response_cfg.h"

int wtk_response_cfg_init(wtk_response_cfg_t *cfg)
{
	wtk_string_set_s(&(cfg->server),"qdreamer");
	cfg->use_server=1;
	cfg->use_times=1;
	cfg->use_stream_length=1;
	cfg->use_hook=1;
	cfg->use_log=1;
	cfg->use_stream_id=1;
	cfg->use_upserver=1;
	cfg->use_audio_url=1;
	cfg->use_test=1;
	cfg->use_custom_hdr=1;
	cfg->use_lua=1;
	cfg->use_stream_message=1;
	cfg->use_stream_mode=1;
	cfg->use_date=1;
	cfg->use_connection=1;
    cfg->use_connection_keep_alive_http1_1=1;
    cfg->use_content_type=1;
    cfg->use_content_type_text_plain=1;
	return 0;
}

int wtk_response_cfg_clean(wtk_response_cfg_t *cfg)
{
	return 0;
}

int wtk_response_cfg_update_local(wtk_response_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_string_v(lc,cfg,server,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_server,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_times,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_stream_length,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_hook,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_log,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_stream_id,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_upserver,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_audio_url,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_test,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_custom_hdr,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_lua,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_stream_message,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_stream_mode,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_date,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_connection,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_connection_keep_alive_http1_1,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_content_type,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_content_type_text_plain,v);

	return 0;
}

int wtk_response_cfg_update(wtk_response_cfg_t *cfg)
{
	return 0;
}
