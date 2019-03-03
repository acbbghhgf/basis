#include "wtk_request.h"
#include "wtk_request_cfg.h"
#include "wtk/core/wtk_strv.h"

static wtk_strv_t hdr_parser[]={
		wtk_strv_s("content-length",wtk_request_update_content_length),
		wtk_strv_s("content-type",wtk_request_update_content_type),
		wtk_strv_s("connection",wtk_request_update_connection),
		wtk_strv_s("stream-id",wtk_request_update_stream_id),
		wtk_strv_s("stream-mode",wtk_request_update_stream_mode),
		wtk_strv_s("script",wtk_request_update_script),
		wtk_strv_s("audio-tag",wtk_request_update_audio_tag),
		wtk_strv_s("log",wtk_request_update_log),
		wtk_strv_s("hook",wtk_request_update_hook),
		wtk_strv_s("user-agent",wtk_request_update_user_agent),
		wtk_strv_s("client",wtk_request_update_client),
};

int wtk_request_cfg_init(wtk_request_cfg_t *cfg)
{
	cfg->redirect_custom_hdr=0;
	cfg->hdr_hash=0;
	cfg->hdr_slot=13;
	cfg->heap_size=4096;
	cfg->buf_size=256;
	cfg->buf_rate=1;
	cfg->body_buf_rate=0.5f;
	wtk_strv_array_sort(hdr_parser,sizeof(hdr_parser)/sizeof(wtk_strv_t));
	cfg->use_hash=1;
	cfg->use_times=1;
	return 0;
}

int wtk_request_cfg_clean(wtk_request_cfg_t *cfg)
{
	if(cfg->hdr_hash)
	{
		wtk_str_hash_delete(cfg->hdr_hash);
	}
	return 0;
}

int wtk_request_cfg_update_local(wtk_request_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc=main;
	wtk_string_t *v;
	wtk_array_t *a;
	wtk_string_t **strs;
	int i;
	int ret=0;

	wtk_local_cfg_update_cfg_i(lc,cfg,hdr_slot,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,heap_size,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,buf_size,v);
	wtk_local_cfg_update_cfg_f(lc,cfg,buf_rate,v);
	wtk_local_cfg_update_cfg_f(lc,cfg,body_buf_rate,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_hash,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_times,v);
	a=wtk_local_cfg_find_array_s(lc,"redirect_custom_hdr");
	if(a)
	{
		wtk_heap_t *heap=main->heap;
		wtk_request_fromto_t *item;

		cfg->redirect_custom_hdr=wtk_array_new_h(heap,a->nslot,sizeof(wtk_request_fromto_t*));
		strs=(wtk_string_t**)a->slot;
		for(i=0;i<a->nslot;++i)
		{
			lc=wtk_local_cfg_find_lc(main,strs[i]->data,strs[i]->len);
			//wtk_debug("[%.*s]=%p.\n",strs[i]->len,strs[i]->data,lc);
			if(!lc)
			{
				wtk_debug("[%d/%d:%.*s] not found.\n",i,a->nslot,strs[i]->len,strs[i]->data);
				goto end;
			}
			item=(wtk_request_fromto_t*)wtk_heap_malloc(heap,sizeof(wtk_request_fromto_t));
			item->from=item->to=0;
			wtk_local_cfg_update_cfg_string(lc,item,from,v);
			wtk_local_cfg_update_cfg_string(lc,item,to,v);
			if(!item->from || !item->to)
			{
				ret=-1;
				goto end;
			}
			*((wtk_request_fromto_t**)wtk_array_push(cfg->redirect_custom_hdr))=item;
		}
	}
	ret=0;
end:
	return ret;
}

void wtk_request_cfg_add_hdr_parse(wtk_request_cfg_t *cfg)
{
	wtk_str_hash_t *hash=cfg->hdr_hash;

	wtk_str_hash_add_s(hash,"content-length",wtk_request_update_content_length);
	wtk_str_hash_add_s(hash,"content-type",wtk_request_update_content_type);
	wtk_str_hash_add_s(hash,"connection",wtk_request_update_connection);
	wtk_str_hash_add_s(hash,"stream-id",wtk_request_update_stream_id);
	wtk_str_hash_add_s(hash,"stream-mode",wtk_request_update_stream_mode);
	wtk_str_hash_add_s(hash,"script",wtk_request_update_script);
	wtk_str_hash_add_s(hash,"audio-tag",wtk_request_update_audio_tag);
	wtk_str_hash_add_s(hash,"hook",wtk_request_update_hook);
	wtk_str_hash_add_s(hash,"log",wtk_request_update_log);
    wtk_str_hash_add_s(hash,"user-agent",wtk_request_update_user_agent);
    wtk_str_hash_add_s(hash,"client",wtk_request_update_client);
}

int wtk_request_cfg_update(wtk_request_cfg_t *cfg)
{
	if(cfg->use_hash)
	{
		cfg->hdr_hash=wtk_str_hash_new(cfg->hdr_slot);
		wtk_request_cfg_add_hdr_parse(cfg);
	}
	return 0;
}

wtk_request_hdr_parse_f wtk_request_cfg_find_hdr_parse(wtk_request_cfg_t *cfg,char *k,int len)
{
	if(cfg->use_hash)
	{
		return (wtk_request_hdr_parse_f)wtk_str_hash_find(cfg->hdr_hash,k,len);
	}else
	{
		wtk_strv_t *v;

		v=wtk_strv_array_find(hdr_parser,sizeof(hdr_parser)/sizeof(wtk_strv_t),k,len);
		//wtk_debug("[%.*s]=%p\n",k_len,k,v);
		return (wtk_request_hdr_parse_f)(v?v->v.v:0);
	}
}
