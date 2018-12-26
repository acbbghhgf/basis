#include "wtk_lfs_cfg.h"

int wtk_lfs_cfg_init(wtk_lfs_cfg_t *cfg)
{
	wtk_string_set(&(cfg->dir),0,0);
	wtk_string_set_s(&(cfg->url),"/*.*");
	cfg->buf_size=1024*1024;
	cfg->buf_rate=1;
	cfg->heap=0;
	return 0;
}

int wtk_lfs_cfg_clean(wtk_lfs_cfg_t *cfg)
{
	return 0;
}

int wtk_lfs_cfg_update_local(wtk_lfs_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	cfg->heap=lc->heap;
	wtk_local_cfg_update_cfg_string_v(lc,cfg,dir,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,url,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,buf_size,v);
	wtk_local_cfg_update_cfg_f(lc,cfg,buf_rate,v);
	return 0;
}

int wtk_lfs_cfg_update(wtk_lfs_cfg_t *cfg)
{
	wtk_strbuf_t *buf;

	if(!cfg->heap){return 0;}
	buf=wtk_strbuf_new(256,1);
	wtk_real_fn(cfg->dir.data,cfg->dir.len,buf,DIR_SEP);
	wtk_heap_fill_string(cfg->heap,&(cfg->dir),buf->data,buf->pos);
	wtk_strbuf_delete(buf);
	return 0;
}
