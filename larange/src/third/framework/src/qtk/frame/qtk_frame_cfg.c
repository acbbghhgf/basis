#include "qtk_frame_cfg.h"
#include "wtk/os/wtk_proc.h"
#include "wtk/os/wtk_log.h"


void qtk_fwd_cfg_set_fn(qtk_fwd_cfg_t* cfg)
{
    wtk_string_set_s(&(cfg->log_fn),"framework.log");
    wtk_string_set_s(&(cfg->pid_fn),"framework.pid");
	return;
}

int qtk_fwd_cfg_init(qtk_fwd_cfg_t *cfg)
{
	qtk_fwd_cfg_set_fn(cfg);
	cfg->daemon=0;
	cfg->send_mail=1;
    cfg->ld_deepbind = 0;
    cfg->statis_time = 1000;
    cfg->uplayer = NULL;
    cfg->uplayer_name = NULL;
    qtk_event_cfg_init(&cfg->event);
    qtk_entry_cfg_init(&cfg->entry);
    wtk_string_set(&cfg->mail_tmpl, NULL, 0);
    cfg->mail_buf = wtk_strbuf_new(1024 * 10, 1);
	return 0;
}

int qtk_fwd_cfg_clean(qtk_fwd_cfg_t *cfg)
{
    qtk_event_cfg_clean(&cfg->event);
    qtk_entry_cfg_clean(&cfg->entry);
    if (cfg->mail_buf) {
        wtk_strbuf_delete(cfg->mail_buf);
        cfg->mail_buf = NULL;
    }
    return 0;
}

int qtk_http_cfg_update_depend(qtk_fwd_cfg_t *cfg)
{
	return 0;
}

int qtk_fwd_cfg_update(qtk_fwd_cfg_t *cfg)
{
	int ret;

    ret = qtk_event_cfg_update(&cfg->event);
	if(ret!=0){goto end;}
    ret = qtk_entry_cfg_update(&cfg->entry);
	if(ret!=0){goto end;}
    if (cfg->mail_tmpl.len) {
        wtk_strbuf_read(cfg->mail_buf, cfg->mail_tmpl.data);
        ret = cfg->mail_buf->pos > 0 ? 0 : -1;
        if (ret) goto end;
    }
	ret=qtk_http_cfg_update_depend(cfg);
end:
	return ret;
}


int qtk_fwd_cfg_update_local(qtk_fwd_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc=main;
	wtk_string_t *v;
	int ret=0;

	wtk_local_cfg_update_cfg_string_v(lc,cfg,log_fn,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,pid_fn,v);
    wtk_local_cfg_update_cfg_i(lc, cfg, statis_time, v);
	wtk_local_cfg_update_cfg_b(lc,cfg,daemon,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,ld_deepbind,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,send_mail,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,mail_tmpl,v);
    lc = wtk_local_cfg_find_lc_s(main,"event");
	if(lc)
	{
		ret=qtk_event_cfg_update_local(&(cfg->event),lc);
		if(ret!=0){goto end;}
	}
    lc = wtk_local_cfg_find_lc_s(main,"entry");
	if(lc)
	{
		ret=qtk_entry_cfg_update_local(&(cfg->entry),lc);
		if(ret!=0){goto end;}
	}
    lc = wtk_local_cfg_find_lc_s(main, "uplayer");
    if (lc) {
        cfg->uplayer = lc;
        wtk_local_cfg_update_cfg_str2(lc, cfg, uplayer_name, name, v);
    } else {
        wtk_debug("WANNING: no uplayer\r\n");
    }
	ret=0;
end:
	return ret;
}

void qtk_fwd_cfg_print(qtk_fwd_cfg_t *cfg)
{
	printf("-----  HTTP ---------\n");
	print_cfg_i(cfg,daemon);
    print_cfg_i(cfg, statis_time);
    qtk_event_cfg_print(&(cfg->event));
    qtk_entry_cfg_print(&(cfg->entry));
}
