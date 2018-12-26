#include "wtk_faqc.h" 
int wtk_faqc_run(wtk_faqc_t *c,wtk_thread_t *t);

wtk_faqc_t* wtk_faqc_new(wtk_faqc_cfg_t *cfg)
{
	wtk_faqc_t *c;

	c=(wtk_faqc_t*)wtk_malloc(sizeof(wtk_faqc_t));
	c->cfg=cfg;
	c->httpc=wtk_httpc_new(&(cfg->httpc));
	c->run=1;
	c->buf=wtk_strbuf_new(256,1);
	c->cur_conf=0.0f;
	wtk_sem_init(&(c->r_sem),0);
	wtk_thread_init(&(c->thread),(thread_route_handler)wtk_faqc_run,c);
	wtk_thread_set_name(&(c->thread),"faqc");
	wtk_thread_start(&(c->thread));
	return c;
}

void wtk_faqc_delete(wtk_faqc_t *faq)
{
	faq->run=0;
	wtk_sem_release(&(faq->r_sem),1);
	wtk_thread_join(&(faq->thread));
	wtk_thread_clean(&(faq->thread));
	wtk_httpc_delete(faq->httpc);
	wtk_strbuf_delete(faq->buf);
	wtk_sem_clean(&(faq->r_sem));
	wtk_free(faq);
}

int wtk_faqc_run(wtk_faqc_t *c,wtk_thread_t *t)
{
	wtk_strbuf_t *buf=c->buf;
	int ret;
	wtk_strbuf_t *tmp;
	int pos;
	wtk_string_t v;
	wtk_json_parser_t *psr;

	psr=wtk_json_parser_new();
	tmp=wtk_strbuf_new(256,1);
	//question=你叫什么名字啊&appid=1456297763827
	wtk_strbuf_push_s(tmp,"appid=");
	wtk_strbuf_push(tmp,c->cfg->appid.data,c->cfg->appid.len);
	wtk_strbuf_push_s(tmp,"&question=");
	pos=tmp->pos;
	while(c->run)
	{
		//wtk_debug("wait ...\n");
		wtk_sem_acquire(&(c->r_sem),-1);
		if(!c->run){break;}
		tmp->pos=pos;
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		wtk_http_url_encode(tmp,buf->data,buf->pos);
		//wtk_debug("[%.*s]\n",tmp->pos,tmp->data);
		ret = wtk_httpc_req(c->httpc,tmp->data,tmp->pos);
		//change by pfc123
		//no long connection,faild access happened when last access is long ago
		if(ret != 0)
		{
			ret = wtk_httpc_req(c->httpc,tmp->data,tmp->pos);
		}
		v=wtk_httpc_get(c->httpc);
//		wtk_debug("======v[%.*s]\n", v.len, v.data);
		wtk_strbuf_reset(buf);
		if(v.len>0)
		{
			wtk_json_parser_reset(psr);
			ret=wtk_json_parser_parse(psr,v.data,v.len);
			if(ret==0)
			{
				wtk_json_item_t *item;
				float conf;

				conf=0.0f;
				item=wtk_json_obj_get_s(psr->json->main,"answer");
				if(item && item->v.str)
				{
					wtk_strbuf_push(buf,item->v.str->data,item->v.str->len);
				}
				item=wtk_json_obj_get_s(psr->json->main,"conf");
				if (item && item->v.number){
					conf = (float)item->v.number;
				}
				c->cur_conf = conf;
				//wtk_debug("answer=%.*s conf=%lf\n", buf->pos, buf->data, conf);
			}
		}
		wtk_sem_release(&(t->sem),1);
	}
	wtk_json_parser_delete(psr);
	wtk_strbuf_delete(tmp);
	return 0;
}

void wtk_faqc_query_start(wtk_faqc_t *faq,char *data,int bytes)
{
	//wtk_debug("start ...\n");
	wtk_strbuf_reset(faq->buf);
	wtk_strbuf_push(faq->buf,data,bytes);
	wtk_sem_release(&(faq->r_sem),1);
}

wtk_string_t wtk_faqc_get_query2(wtk_faqc_t *faq, float* conf)
{
	wtk_string_t v;
	int ret;

	ret=wtk_sem_acquire(&(faq->thread.sem),-1);
	if(ret==0)
	{
		wtk_string_set(&(v),faq->buf->data,faq->buf->pos);
	}else
	{
		wtk_string_set(&(v),0,0);
	}
	*conf = faq->cur_conf;

	return v;
}

wtk_string_t wtk_faqc_get_query(wtk_faqc_t *faq)
{
	wtk_string_t v;
	int ret;

	ret=wtk_sem_acquire(&(faq->thread.sem),-1);
	if(ret==0)
	{
		wtk_string_set(&(v),faq->buf->data,faq->buf->pos);
	}else
	{
		wtk_string_set(&(v),0,0);
	}

	return v;
}
