#include "wtk_nlpemot_cfg.h" 

int wtk_nlpemot_cfg_init(wtk_nlpemot_cfg_t *cfg)
{
	wtk_segmenter_cfg_init(&(cfg->segmenter));
	cfg->dict_fn=NULL;
	cfg->not_fn=NULL;
	cfg->verry_fn=NULL;
	cfg->not_hash=NULL;
	cfg->verry_hash=NULL;
	cfg->min=0;
	cfg->max=10;
	cfg->sep=NULL;
	cfg->lex_fn=NULL;
	cfg->lex_net=NULL;
	return 0;
}

int wtk_nlpemot_cfg_clean(wtk_nlpemot_cfg_t *cfg)
{
	if(cfg->lex_net)
	{
		wtk_lex_net_delete(cfg->lex_net);
	}
	if(cfg->not_hash)
	{
		wtk_str_hash_delete(cfg->not_hash);
	}
	if(cfg->verry_hash)
	{
		wtk_str_hash_delete(cfg->verry_hash);
	}
	wtk_segmenter_cfg_clean(&(cfg->segmenter));
	return 0;
}

int wtk_nlpemot_cfg_update_local(wtk_nlpemot_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;
	int ret;

	lc=main;
	//wtk_local_cfg_print(lc);
	wtk_local_cfg_update_cfg_str(lc,cfg,lex_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,dict_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,not_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,verry_fn,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,min,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,max,v);
	cfg->sep=wtk_local_cfg_find_array_s(lc,"sep");
	lc=wtk_local_cfg_find_lc_s(main,"segmenter");
	//wtk_debug("lc=%p\n",lc);
	if(lc)
	{
		ret=wtk_segmenter_cfg_update_local(&(cfg->segmenter),lc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

int wtk_nlpemot_cfg_load_not(wtk_nlpemot_cfg_t *cfg,wtk_source_t *src)
{
	wtk_strbuf_t *buf;
	int ret;
	wtk_string_t *v;
	wtk_str_hash_t *hash;
	int eof;

	cfg->not_hash=wtk_str_hash_new(71);
	hash=cfg->not_hash;
	buf=wtk_strbuf_new(256,1);
	while(1)
	{
		ret=wtk_source_skip_sp2(src,NULL,&eof);
		if(eof){ret=0;goto end;}
		ret=wtk_source_read_string(src,buf);
		if(ret!=0){ret=0;goto end;}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		v=wtk_heap_dup_string(hash->heap,buf->data,buf->pos);
		wtk_str_hash_add(hash,v->data,v->len,v);
	}
	ret=0;
end:
	wtk_strbuf_delete(buf);
	return ret;
}

int wtk_nlpemot_cfg_load_verry(wtk_nlpemot_cfg_t *cfg,wtk_source_t *src)
{
	wtk_strbuf_t *buf;
	int ret;
	wtk_str_hash_t *hash;
	wtk_hash_str_node_t *node;
	float f;
	int eof;

	cfg->verry_hash=wtk_str_hash_new(171);
	hash=cfg->verry_hash;
	buf=wtk_strbuf_new(256,1);
	while(1)
	{
		ret=wtk_source_skip_sp2(src,NULL,&eof);
		if(eof){ret=0;goto end;}
		ret=wtk_source_read_string(src,buf);
		if(ret!=0){ret=0;goto end;}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		ret=wtk_source_read_float(src,&f,1,0);
		if(ret!=0){goto end;}
		node=(wtk_hash_str_node_t*)wtk_str_hash_find_node3(hash,buf->data,buf->pos,1);
		node->v.f=f;
		//wtk_debug("[%.*s]=%f\n",buf->pos,buf->data,f);
	}
	ret=0;
end:
	wtk_strbuf_delete(buf);
	return ret;
}


int wtk_nlpemot_cfg_update(wtk_nlpemot_cfg_t *cfg,wtk_lexc_t *lex)
{
	wtk_source_loader_t sl;

	sl.hook=0;
	sl.vf=wtk_source_load_file_v;
	return wtk_nlpemot_cfg_update2(cfg,&(sl),lex);
}

int wtk_nlpemot_cfg_update2(wtk_nlpemot_cfg_t *cfg,wtk_source_loader_t *sl,wtk_lexc_t *lex)
{
	int ret;

	ret=wtk_segmenter_cfg_update2(&(cfg->segmenter),sl);
	if(ret!=0)
	{
		wtk_debug("update segmenter failed\n");
		goto end;
	}
	if(cfg->not_fn)
	{
		ret=wtk_source_loader_load(sl,cfg,(wtk_source_load_handler_t)wtk_nlpemot_cfg_load_not,cfg->not_fn);
		if(ret!=0)
		{
			wtk_debug("update not failed\n");
			goto end;
		}
	}
	if(cfg->verry_fn)
	{
		ret=wtk_source_loader_load(sl,cfg,(wtk_source_load_handler_t)wtk_nlpemot_cfg_load_verry,cfg->verry_fn);
		if(ret!=0)
		{
			wtk_debug("update verry failed\n");
			goto end;
		}
	}
	if(cfg->lex_fn)
	{
		wtk_rbin2_t *rb=(wtk_rbin2_t*)sl->hook;

		lex->rbin=rb;
		cfg->lex_net=wtk_lexc_compile_file2(lex,cfg->lex_fn);
		if(!cfg->lex_net)
		{
			wtk_debug("compile file[%s] failed\n",cfg->lex_fn);
			ret=-1;
			goto end;
		}
	}
	ret=0;
end:
	return ret;
}
