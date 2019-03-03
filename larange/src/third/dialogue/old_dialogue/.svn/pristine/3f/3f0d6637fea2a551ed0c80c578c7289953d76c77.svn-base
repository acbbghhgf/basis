#include "wtk_tfidf_cfg.h"
#include "wtk/core/cfg/wtk_source.h"

int wtk_tfidf_cfg_init(wtk_tfidf_cfg_t *cfg)
{
	wtk_segmenter_cfg_init(&(cfg->segmenter));
	cfg->idf_thresh=0;

	cfg->nbest=10;
	cfg->def.len=0;
	cfg->skip_ws=1;
	cfg->use_seg=1;
	cfg->sym_fn=NULL;
	cfg->char_map=NULL;
	cfg->stop_fn=NULL;
	cfg->stop_map=NULL;

	cfg->bin_fn=NULL;
	return 0;
}

int wtk_tfidf_cfg_clean(wtk_tfidf_cfg_t *cfg)
{
	if(cfg->stop_map)
	{
		wtk_str_hash_delete(cfg->stop_map);
	}
	if(cfg->char_map)
	{
		wtk_str_hash_delete(cfg->char_map);
	}
	wtk_segmenter_cfg_clean(&(cfg->segmenter));
	return 0;
}

int wtk_tfidf_cfg_update_local(wtk_tfidf_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc=main;
	wtk_string_t *v;
	int ret=-1;

	wtk_local_cfg_update_cfg_f(lc,cfg,idf_thresh,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,def,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,bin_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,sym_fn,v)
	wtk_local_cfg_update_cfg_str(lc,cfg,stop_fn,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,nbest,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,skip_ws,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_seg,v);
	lc=wtk_local_cfg_find_lc_s(main,"segmenter");
	if(lc)
	{
		ret=wtk_segmenter_cfg_update_local(&(cfg->segmenter),lc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

int wtk_tfidf_cfg_load_sym(wtk_tfidf_cfg_t *sym,wtk_source_t *s)
{
	wtk_strbuf_t *buf;
	int ret;
	wtk_str_hash_t *hash;
	wtk_hash_str_node_t *node;
	int id=0;

	sym->char_map=hash=wtk_str_hash_new(2507);
	buf=wtk_strbuf_new(256,1);
	while(1)
	{
		ret=wtk_source_read_string(s,buf);
		if(ret!=0){ret=0;goto end;}
		node=(wtk_hash_str_node_t*)wtk_str_hash_find_node3(hash,buf->data,buf->pos,1);
		node->v.u=id;
		++id;
	}
	ret=0;
end:
	wtk_strbuf_delete(buf);
	return ret;
}

int wtk_tfidf_cfg_load_stop(wtk_tfidf_cfg_t *sym,wtk_source_t *s)
{
	wtk_strbuf_t *buf;
	int ret;
	wtk_str_hash_t *hash;
	wtk_hash_str_node_t *node;
	int id=0;

	sym->stop_map=hash=wtk_str_hash_new(1107);
	buf=wtk_strbuf_new(256,1);
	while(1)
	{
		ret=wtk_source_read_string(s,buf);
		if(ret!=0){ret=0;goto end;}
		node=(wtk_hash_str_node_t*)wtk_str_hash_find_node3(hash,buf->data,buf->pos,1);
		node->v.u=id;
		++id;
	}
	ret=0;
end:
	wtk_strbuf_delete(buf);
	return ret;
}



int wtk_tfidf_cfg_update(wtk_tfidf_cfg_t *cfg)
{
	int ret;

	if(cfg->stop_fn)
	{
		ret=wtk_source_load_file(cfg,(wtk_source_load_handler_t)wtk_tfidf_cfg_load_stop,cfg->stop_fn);
		if(ret!=0){goto end;}
	}
	if(cfg->sym_fn)
	{
		ret=wtk_source_load_file(cfg,(wtk_source_load_handler_t)wtk_tfidf_cfg_load_sym,cfg->sym_fn);
		if(ret!=0){goto end;}
	}
	if(cfg->use_seg)
	{
		ret=wtk_segmenter_cfg_update(&(cfg->segmenter));
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}
