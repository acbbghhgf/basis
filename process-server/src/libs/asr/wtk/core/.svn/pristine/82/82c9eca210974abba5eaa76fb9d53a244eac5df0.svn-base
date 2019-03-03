#include "wtk/core/cfg/wtk_source.h"
#include "wtk_segmenter_cfg.h"

int wtk_segmenter_cfg_init(wtk_segmenter_cfg_t *cfg)
{
	cfg->dict_fn=0;
	cfg->nslot=1154173;
	cfg->hash=0;
	cfg->def_weight=40;
	cfg->filter=0;
	cfg->max_char=-1;
	cfg->upper=0;
	cfg->lower=0;
	cfg->max_line=-1;
	cfg->use_bin=0;
	wtk_string_set_s(&(cfg->pre),"<s> ");
	wtk_string_set_s(&(cfg->suf)," </s>");
	return 0;
}

int wtk_segmenter_cfg_clean(wtk_segmenter_cfg_t *cfg)
{
	if(cfg->hash)
	{
		wtk_str_hash_delete(cfg->hash);
	}
	//wtk_fkv2_delete(cfg->kv.fkv);
	return 0;
}

int wtk_segmenter_cfg_update_local(wtk_segmenter_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;

	lc=main;
	cfg->filter=wtk_local_cfg_find_array_s(lc,"filter");
	wtk_local_cfg_update_cfg_str(lc,cfg,dict_fn,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,pre,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,suf,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,nslot,v);
	wtk_local_cfg_update_cfg_f(lc,cfg,def_weight,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,max_char,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,upper,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,lower,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_bin,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,max_line,v);
	return 0;
}

wtk_segwrd_t* wtk_segwrd_new(wtk_heap_t *heap,char *wrd,int wrd_bytes,double prob)
{
	wtk_segwrd_t *w;

	w=(wtk_segwrd_t*)wtk_heap_malloc(heap,sizeof(*w));
	w->wrd=wtk_heap_dup_string(heap,wrd,wrd_bytes);
	w->prob=prob;
	return w;
}

void wtk_segwrd_print(wtk_segwrd_t *w)
{
	printf("[%.*s]=%f\n",w->wrd->len,w->wrd->data,w->prob);
}

int wtk_segmenter_cfg_load_dict(wtk_segmenter_cfg_t *cfg,wtk_source_t *src)
{
	wtk_strbuf_t *buf;
	double prob;
	wtk_segwrd_t *w;
	wtk_str_hash_t *hash=cfg->hash;
	wtk_heap_t *heap=hash->heap;
	int ret=-1;

	buf=wtk_strbuf_new(256,1);
	while(1)
	{
		//ret=wtk_source_seek_to2_s(src,"	",buf);
		//print_data(buf->data,buf->pos);
		//wtk_debug("ret=%d\n",ret);
		//if(ret!=0){goto end;}
		ret=wtk_source_read_normal_string(src,buf);
		wtk_strbuf_strip(buf);
		if(ret!=0)
		{
			ret=0;
			//wtk_debug("read word failed[%.*s].\n",buf->pos,buf->data);
			goto end;
		}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		ret=wtk_source_read_double(src,&prob,1);
		if(ret!=0)
		{
			wtk_debug("read [%.*s] prob failed.\n",buf->pos,buf->data);
			goto end;
		}
		//wtk_debug("[%.*s]=%f\n",buf->pos,buf->data,prob);
		w=wtk_segwrd_new(heap,buf->data,buf->pos,prob);
		wtk_str_hash_add(hash,w->wrd->data,w->wrd->len,w);
		wtk_source_read_line(src,buf);
	}
end:
	//exit(0);
	wtk_strbuf_delete(buf);
	return ret;
}

int wtk_segmenter_cfg_update(wtk_segmenter_cfg_t *cfg)
{
	int ret;

	if(cfg->dict_fn)
	{
		if(cfg->use_bin)
		{
//			cfg->kv.fkv=wtk_fkv2_new(cfg->dict_fn);
//			ret=cfg->kv.fkv?0:-1;
			ret=0;
		}else
		{
			if(cfg->hash)
			{
				wtk_str_hash_delete(cfg->hash);
			}
			cfg->hash=wtk_str_hash_new(cfg->nslot);
			ret=wtk_source_load_file(cfg,(wtk_source_load_handler_t)wtk_segmenter_cfg_load_dict,cfg->dict_fn);
		}
	}else
	{
		ret=0;
	}
	return ret;
}


int wtk_segmenter_cfg_update2(wtk_segmenter_cfg_t *cfg,wtk_source_loader_t *sl)
{
	int ret;

	if(cfg->dict_fn)
	{
		if(cfg->use_bin)
		{
			ret=0;
		}else
		{
			if(cfg->hash)
			{
				wtk_str_hash_delete(cfg->hash);
			}
			cfg->hash=wtk_str_hash_new(cfg->nslot);
//			ret=wtk_source_load_file(cfg,(wtk_source_load_handler_t)wtk_segmenter_cfg_load_dict,cfg->dict_fn);
			ret=wtk_source_loader_load(sl,cfg,(wtk_source_load_handler_t)wtk_segmenter_cfg_load_dict,cfg->dict_fn);
		}
	}else
	{
		ret=0;
	}
	return ret;
}
