#ifdef USE_CRF
#include "wtk_crfact_parser.h"
#include <ctype.h>
#include <math.h>

wtk_crfact_parser_t* wtk_crfact_parser_new(wtk_crfact_parser_cfg_t *cfg)
{
	wtk_crfact_parser_t *p;

	p=(wtk_crfact_parser_t*)wtk_malloc(sizeof(wtk_crfact_parser_t));
	p->cfg=cfg;
	p->seg=wtk_poseg_new(&(cfg->poseg));
	p->crf=wtk_crf_new(cfg->model);
	p->buf=wtk_strbuf_new(256,1);
	p->heap=wtk_heap_new(4096);
	return p;
}

void wtk_crfact_parser_delete(wtk_crfact_parser_t *p)
{
	wtk_heap_delete(p->heap);
	wtk_strbuf_delete(p->buf);
	wtk_crf_delete(p->crf);
	wtk_poseg_delete(p->seg);
	wtk_free(p);
}

void wtk_crfact_parser_reset(wtk_crfact_parser_t *p)
{
	wtk_heap_reset(p->heap);
}

wtk_crfact_item_t* wtk_crfact_item_new(wtk_heap_t *heap);
wtk_crfact_value_t* wtk_crfact_value_new(wtk_heap_t *heap,wtk_string_t *wrd,wtk_string_t *pos);

wtk_crfact_t* wtk_crfact_new(wtk_heap_t *heap)
{
	wtk_crfact_t *act;

	act=(wtk_crfact_t*)wtk_heap_malloc(heap,sizeof(wtk_crfact_t));
	wtk_queue_init(&(act->act_item_q));
	act->item=NULL;
	return act;
}

void wtk_crfact_set(wtk_crfact_t *act,wtk_heap_t *heap,char *k,int k_bytes,char *v,int v_bytes)
{
	wtk_crfact_value_t *value;

	//wtk_debug("[%.*s]=[%.*s]\n",k_bytes,k,v_bytes,v);
	if(!act->item)
	{
		act->item=wtk_crfact_item_new(heap);
		wtk_queue_push(&(act->act_item_q),&(act->item->q_n));
	}
	value=wtk_crfact_value_new(heap,NULL,NULL);
	value->wrd=wtk_heap_dup_string(heap,v,v_bytes);
	if(wtk_str_equal_s(k,k_bytes,"type") || wtk_str_equal_s(k,k_bytes,"act"))
	{
		act->item->type=value->wrd;
		act->item->type_value=value;
	}else if(wtk_str_equal_s(k,k_bytes,"ap"))
	{
		if(!act->item->ap)
		{
			act->item->ap=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
			wtk_queue_init(act->item->ap);
		}
		wtk_queue_push(act->item->ap,&(value->q_n));
	}else if(wtk_str_equal_s(k,k_bytes,"p"))
	{
		act->item->p=value;
	}else if(wtk_str_equal_s(k,k_bytes,"vtd"))
	{
		if(!act->item->vtd)
		{
			act->item->vtd=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
			wtk_queue_init(act->item->vtd);
		}
		wtk_queue_push(act->item->vtd,&(value->q_n));
	}else if(wtk_str_equal_s(k,k_bytes,"vt1"))
	{
		if(!act->item->vt1)
		{
			act->item->vt1=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
			wtk_queue_init(act->item->vt1);
		}
		wtk_queue_push(act->item->vt1,&(value->q_n));
	}else if(wtk_str_equal_s(k,k_bytes,"vt"))
	{
		act->item->vt=value;
	}else if(wtk_str_equal_s(k,k_bytes,"at"))
	{
		if(!act->item->at)
		{
			act->item->at=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
			wtk_queue_init(act->item->at);
		}
		wtk_queue_push(act->item->at,&(value->q_n));
	}else if(wtk_str_equal_s(k,k_bytes,"st"))
	{
		if(!act->item->st)
		{
			act->item->st=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
			wtk_queue_init(act->item->st);
		}
		wtk_queue_push(act->item->st,&(value->q_n));
	}else if(wtk_str_equal_s(k,k_bytes,"t"))
	{
		act->item->t=value;
	}else if(wtk_str_equal_s(k,k_bytes,"apn"))
	{
		act->item->apn=value;
	}else if(wtk_str_equal_s(k,k_bytes,"pn"))
	{
		act->item->pn=value;
	}else if(wtk_str_equal_s(k,k_bytes,"pa"))
	{
		act->item->pa=value;
	}else if(wtk_str_equal_s(k,k_bytes,"pvd"))
	{
		if(!act->item->pvd)
		{
			act->item->pvd=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
			wtk_queue_init(act->item->pvd);
		}
		wtk_queue_push(act->item->pvd,&(value->q_n));
	}else if(wtk_str_equal_s(k,k_bytes,"pv1"))
	{
		if(!act->item->pv1)
		{
			act->item->pv1=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
			wtk_queue_init(act->item->pv1);
		}
		wtk_queue_push(act->item->pv1,&(value->q_n));
	}else if(wtk_str_equal_s(k,k_bytes,"pv"))
	{
		act->item->pv=value;
	}else if(wtk_str_equal_s(k,k_bytes,"rt"))
	{
		act->item->rt=value;
	}else if(wtk_str_equal_s(k,k_bytes,"is"))
	{
		act->item->is=value;
	}
}

int wtk_crfact_update_value(wtk_crfact_t *act,wtk_heap_t *heap,wtk_string_t *k,wtk_crfact_value_t *value)
{
	if(!act->item)
	{
		act->item=wtk_crfact_item_new(heap);
		wtk_queue_push(&(act->act_item_q),&(act->item->q_n));
	}
	if(wtk_string_cmp_s(k,"type")==0)
	{
		act->item->type_value=value;
	}else if(wtk_string_cmp_s(k,"ap")==0)
	{
		if(!act->item->ap)
		{
			act->item->ap=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
			wtk_queue_init(act->item->ap);
		}
		wtk_queue_push(act->item->ap,&(value->q_n));
	}else if(wtk_string_cmp_s(k,"p")==0)
	{
		act->item->p=value;
	}else if(wtk_string_cmp_s(k,"vtd")==0)
	{
		if(!act->item->vtd)
		{
			act->item->vtd=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
			wtk_queue_init(act->item->vtd);
		}
		wtk_queue_push(act->item->vtd,&(value->q_n));
	}else if(wtk_string_cmp_s(k,"vt1")==0)
	{
		if(!act->item->vt1)
		{
			act->item->vt1=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
			wtk_queue_init(act->item->vt1);
		}
		wtk_queue_push(act->item->vt1,&(value->q_n));
	}else if(wtk_string_cmp_s(k,"vt")==0)
	{
		act->item->vt=value;
	}else if(wtk_string_cmp_s(k,"at")==0)
	{
		if(!act->item->at)
		{
			act->item->at=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
			wtk_queue_init(act->item->at);
		}
		wtk_queue_push(act->item->at,&(value->q_n));
	}else if(wtk_string_cmp_s(k,"st")==0)
	{
		if(!act->item->st)
		{
			act->item->st=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
			wtk_queue_init(act->item->st);
		}
		wtk_queue_push(act->item->st,&(value->q_n));
	}else if(wtk_string_cmp_s(k,"t")==0)
	{
		act->item->t=value;
	}else if(wtk_string_cmp_s(k,"apn")==0)
	{
		act->item->apn=value;
	}else if(wtk_string_cmp_s(k,"pn")==0)
	{
		act->item->pn=value;
	}else if(wtk_string_cmp_s(k,"pa")==0)
	{
		act->item->pa=value;
	}else if(wtk_string_cmp_s(k,"pvd")==0)
	{
		if(!act->item->pvd)
		{
			act->item->pvd=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
			wtk_queue_init(act->item->pvd);
		}
		wtk_queue_push(act->item->pvd,&(value->q_n));
	}else if(wtk_string_cmp_s(k,"pv1")==0)
	{
		if(!act->item->pv1)
		{
			act->item->pv1=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
			wtk_queue_init(act->item->pv1);
		}
		wtk_queue_push(act->item->pv1,&(value->q_n));
	}else if(wtk_string_cmp_s(k,"pv")==0)
	{
		act->item->pv=value;
	}else if(wtk_string_cmp_s(k,"rt")==0)
	{
		act->item->rt=value;
	}else if(wtk_string_cmp_s(k,"is")==0)
	{
		act->item->is=value;
	}
	else
	{
		wtk_debug("[%.*s] not found.\n",k->len,k->data);
		return -1;
	}
	return 0;
}

wtk_crfact_t* wtk_crfact_new2(wtk_heap_t *heap,char *s,int s_bytes)
{
typedef enum
{
	WTK_CRFACT_INIT,
	WTK_CRFACT_TYPE,
	WTK_CRFACT_WAIT_BRACE,
	WTK_CRFACT_WAIT_KV,
	WTK_CRFACT_K,
	WTK_CRFACT_WAIT_V,
	WTK_CRFACT_K_END,
	WTK_CRFACT_V,
	WTK_CRFACT_V_QUOTE,
	WTK_CRFACT_V_QUOTE_ESC,
	WTK_CRFACT_V_ATTR,
	WTK_CRFACT_V_ATTR_END,
}wtk_crfact_state_t;
	wtk_crfact_t *act;
	char *e;
	int n;
	wtk_crfact_state_t state;
	wtk_crfact_item_t *item;
	wtk_strbuf_t *buf;
	wtk_string_t *k=NULL;
	wtk_crfact_value_t *value=NULL;
	wtk_strkv_parser_t kvp;
	int ret;

	//wtk_debug("[%.*s]\n",s_bytes,s);
	buf=wtk_strbuf_new(256,1);
	act=wtk_crfact_new(heap);
	item=wtk_crfact_item_new(heap);
	act->item=item;
	wtk_queue_push(&(act->act_item_q),&(item->q_n));
	e=s+s_bytes;
	state=WTK_CRFACT_INIT;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		//wtk_debug("[%.*s]=%d\n",n,s,state);
		switch(state)
		{
		case WTK_CRFACT_INIT:
			if(n>1|| !isspace(*s))
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push(buf,s,n);
				state=WTK_CRFACT_TYPE;
			}
			break;
		case WTK_CRFACT_TYPE:
			if(n==1 && (*s=='(' || isspace(*s)))
			{
				//wtk_debug("[%.*s]\n",buf->pos,buf->data);
				item->type=wtk_heap_dup_string(heap,buf->data,buf->pos);
				if(*s=='(')
				{
					state=WTK_CRFACT_WAIT_KV;
				}else
				{
					state=WTK_CRFACT_WAIT_BRACE;
				}
			}else
			{
				wtk_strbuf_push(buf,s,n);
			}
			break;
		case WTK_CRFACT_WAIT_BRACE:
			if(n==1 && *s=='(')
			{
				state=WTK_CRFACT_WAIT_KV;
			}
			break;
		case WTK_CRFACT_WAIT_KV:
			if(n>1 || !isspace(*s))
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push(buf,s,n);
				state=WTK_CRFACT_K;
			}
			break;
		case WTK_CRFACT_K:
			if(n==1 && (*s=='=' || *s==',' || *s==')' || isspace(*s)))
			{
				switch(*s)
				{
				case '=':
					k=wtk_heap_dup_string(heap,buf->data,buf->pos);
					state=WTK_CRFACT_WAIT_V;
					break;
				case ',':
					state=WTK_CRFACT_WAIT_KV;
					break;
				case ')':
					goto end;
					break;
				default:
					state=WTK_CRFACT_K_END;
					break;
				}
			}else
			{
				wtk_strbuf_push(buf,s,n);
			}
			break;
		case WTK_CRFACT_K_END:
			if(n==1 && (*s=='=' || *s==',' || *s==')'))
			{
				switch(*s)
				{
				case '=':
					k=wtk_heap_dup_string(heap,buf->data,buf->pos);
					state=WTK_CRFACT_WAIT_V;
					break;
				case ',':
					state=WTK_CRFACT_WAIT_KV;
					break;
				case ')':
					goto end;
					break;
				default:
					break;
				}
			}
			break;
		case WTK_CRFACT_WAIT_V:
			if(n>1 || !isspace(*s))
			{
				wtk_strbuf_reset(buf);
				if(*s=='"')
				{
					state=WTK_CRFACT_V_QUOTE;
				}else
				{
					state=WTK_CRFACT_V;
					wtk_strbuf_push(buf,s,n);
				}
			}
			break;
		case WTK_CRFACT_V:
			if(n==1 && (*s==',' || *s==')'))
			{
				value=wtk_crfact_value_new(heap,NULL,NULL);
				value->wrd=wtk_heap_dup_string(heap,buf->data,buf->pos);
				wtk_debug("value=%p\n",value);
				ret=wtk_crfact_update_value(act,heap,k,value);
				if(ret!=0){act=NULL;goto end;}
				if(*s==',')
				{
					state=WTK_CRFACT_WAIT_KV;
				}else
				{
					goto end;
				}
			}else
			{
				wtk_strbuf_push(buf,s,n);
			}
			break;
		case WTK_CRFACT_V_QUOTE:
			//wtk_debug("[%.*s]\n",n,s);
			if(n==1 && (*s=='\\'||*s=='\"'||*s=='/'))
			{
				switch(*s)
				{
				case '\\':
					state=WTK_CRFACT_V_QUOTE_ESC;
					break;
				case '\"':
					state=WTK_CRFACT_K_END;
					value=wtk_crfact_value_new(heap,NULL,NULL);
					wtk_strbuf_strip(buf);
					value->wrd=wtk_heap_dup_string(heap,buf->data,buf->pos);
					//wtk_debug("value=%p\n",value);
					ret=wtk_crfact_update_value(act,heap,k,value);
					if(ret!=0){act=NULL;goto end;}
					//wtk_debug("[%.*s]\n",buf->pos,buf->data);
					break;
				case '/':
					state=WTK_CRFACT_V_ATTR;
					value=wtk_crfact_value_new(heap,NULL,NULL);
					wtk_strbuf_strip(buf);
					value->wrd=wtk_heap_dup_string(heap,buf->data,buf->pos);
					//wtk_debug("value=%p\n",value);
					ret=wtk_crfact_update_value(act,heap,k,value);
					if(ret!=0){act=NULL;goto end;}
					//wtk_debug("[%.*s]\n",buf->pos,buf->data);
					wtk_strbuf_reset(buf);
					break;
				}
			}else
			{
				wtk_strbuf_push(buf,s,n);
			}
			break;
		case WTK_CRFACT_V_QUOTE_ESC:
			state=WTK_CRFACT_V_QUOTE;
			break;
		case WTK_CRFACT_V_ATTR:
			if(n==1 && *s=='/')
			{
				int ret;

				//wtk_debug("[%.*s]\n",buf->pos,buf->data);
				wtk_strkv_parser_init(&(kvp),buf->data,buf->pos);
				while(1)
				{
					ret=wtk_strkv_parser_next(&(kvp));
					if(ret!=0){break;}
					//wtk_debug("[%.*s]=[%.*s]\n",kvp.k.len,kvp.k.data,kvp.v.len,kvp.v.data);
					if(kvp.v.len>0)
					{
						if(wtk_string_cmp_s(&(kvp.k),"pos")==0)
						{
							value->pos=wtk_heap_dup_string(heap,kvp.v.data,kvp.v.len);
						}else if(wtk_string_cmp_s(&(kvp.k),"class")==0)
						{
							value->cls=wtk_heap_dup_string(heap,kvp.v.data,kvp.v.len);
						}else if(wtk_string_cmp_s(&(kvp.k),"inst")==0)
						{
							value->inst=wtk_heap_dup_string(heap,kvp.v.data,kvp.v.len);
						}else
						{
							wtk_debug("[%.*s]=[%.*s]\n",kvp.k.len,kvp.k.data,kvp.v.len,kvp.v.data);
							act=NULL;
							goto end;
						}
					}else
					{
						wtk_debug("unk value [%.*s]\n",kvp.v.len,kvp.v.data);
					}
				}
				state=WTK_CRFACT_V_ATTR_END;
				//exit(0);
			}else
			{
				wtk_strbuf_push(buf,s,n);
			}
			break;
		case WTK_CRFACT_V_ATTR_END:
			if(n>1||!isspace(*s))
			{
				if(n==1&&*s=='\"')
				{
					state=WTK_CRFACT_K_END;
				}else if(n==1 && *s==',')
				{
					wtk_strbuf_reset(buf);
					state=WTK_CRFACT_V_QUOTE;
				}
				else
				{
					wtk_strbuf_reset(buf);
					wtk_strbuf_push(buf,s,n);
					state=WTK_CRFACT_V_QUOTE;
				}
			}
			break;
		}
		s+=n;
	}
end:
	//wtk_crfact_print(act);
	//wtk_debug("act=%p item=%p p=%p v=%p t=%p\n",act,act->item,act->item->p,act->item->v,act->item->t);
	wtk_strbuf_delete(buf);
	return act;
}


wtk_crfact_item_t* wtk_crfact_item_new(wtk_heap_t *heap)
{
	wtk_crfact_item_t *item;

	item=(wtk_crfact_item_t*)wtk_heap_malloc(heap,sizeof(wtk_crfact_item_t));
	item->type=NULL;
	item->type_value=NULL;

	item->ap=NULL;
	item->p=NULL;

	item->vtd=NULL;
	item->vt1=NULL;
	item->vt=NULL;

	item->at=NULL;
	item->t=NULL;
	item->st=NULL;

	item->apn=NULL;
	item->pn=NULL;

	item->pa=NULL;

	item->pv1=NULL;
	item->pvd=NULL;
	item->pv=NULL;

	item->rt=NULL;
	item->is=NULL;
	return item;
}

wtk_crfact_value_t* wtk_crfact_value_new(wtk_heap_t *heap,wtk_string_t *wrd,wtk_string_t *pos)
{
	wtk_crfact_value_t *v;

	v=(wtk_crfact_value_t*)wtk_heap_malloc(heap,sizeof(wtk_crfact_value_t));
	if(wrd)
	{
		v->wrd=wtk_heap_dup_string(heap,wrd->data,wrd->len);
	}else
	{
		v->wrd=NULL;
	}
	if(pos)
	{
		v->pos=wtk_heap_dup_string(heap,pos->data,pos->len);
	}else
	{
		v->pos=NULL;
	}
	v->cls=NULL;
	v->inst=NULL;
	//v->value=NULL;
	return v;
}

void wtk_crfact_value_print(wtk_crfact_value_t *v,wtk_strbuf_t *buf)
{
	wtk_strbuf_push(buf,v->wrd->data,v->wrd->len);
	if(v->pos || v->inst || v->cls)
	{
		wtk_strbuf_push_s(buf,"/");
		if(v->pos)
		{
			wtk_strbuf_push_s(buf,"pos=");
			wtk_strbuf_push(buf,v->pos->data,v->pos->len);
		}
		if(v->inst)
		{
			wtk_strbuf_push_s(buf,",inst=");
			wtk_strbuf_push(buf,v->inst->data,v->inst->len);
		}
		if(v->cls)
		{
			wtk_strbuf_push_s(buf,",class=");
			wtk_strbuf_push(buf,v->cls->data,v->cls->len);
		}
		wtk_strbuf_push_s(buf,"/");
	}
}

void wtk_crfact_value_queue_print(wtk_queue_t *q,wtk_strbuf_t *buf)
{
	wtk_queue_node_t *qn;
	wtk_crfact_value_t *v;

	for(qn=q->pop;qn;qn=qn->next)
	{
		v=data_offset2(qn,wtk_crfact_value_t,q_n);
		if(qn!=q->pop)
		{
			wtk_strbuf_push_s(buf,",");
		}
		wtk_crfact_value_print(v,buf);
	}
}


void wtk_crfact_item_print2(wtk_crfact_item_t *v,wtk_strbuf_t *buf)
{
	int pad=0;

	wtk_strbuf_reset(buf);
	if(v->type)
	{
		wtk_strbuf_push_f(buf,"%.*s(",v->type->len,v->type->data);
		if(v->type_value)
		{
			wtk_strbuf_push_f(buf,"type=\"");
			wtk_crfact_value_print(v->type_value,buf);
			wtk_strbuf_push_f(buf,"\"");
			pad=1;
		}
	}else
	{
		wtk_strbuf_push_f(buf,"i(");
	}
	//wtk_strbuf_push_f(buf,"(");
	//wtk_debug("ap=%p\n",v->ap);
	if(v->ap)
	{
		if(pad)
		{
			wtk_strbuf_push_f(buf,",");
		}
		wtk_strbuf_push_f(buf,"ap=\"");
		wtk_crfact_value_queue_print(v->ap,buf);
		wtk_strbuf_push_f(buf,"\"");
		pad=1;
	}
	if(v->p)
	{
		if(pad)
		{
			wtk_strbuf_push_f(buf,",");
		}
		wtk_strbuf_push_f(buf,"p=\"");
		wtk_crfact_value_print(v->p,buf);
		wtk_strbuf_push_f(buf,"\"");
		pad=1;
	}
	if(v->vtd)
	{
		if(pad)
		{
			wtk_strbuf_push_f(buf,",");
		}
		wtk_strbuf_push_f(buf,"vtd=\"");
		wtk_crfact_value_queue_print(v->vtd,buf);
		wtk_strbuf_push_f(buf,"\"");
		pad=1;
	}
	if(v->vt1)
	{
		if(pad)
		{
			wtk_strbuf_push_f(buf,",");
		}
		wtk_strbuf_push_f(buf,"vt1=\"");
		wtk_crfact_value_queue_print(v->vt1,buf);
		wtk_strbuf_push_f(buf,"\"");
		pad=1;
	}
	if(v->vt)
	{
		if(pad)
		{
			wtk_strbuf_push_f(buf,",");
		}
		wtk_strbuf_push_f(buf,"vt=\"");
		wtk_crfact_value_print(v->vt,buf);
		wtk_strbuf_push_f(buf,"\"");
		pad=1;
	}
	if(v->at)
	{
		if(pad)
		{
			wtk_strbuf_push_f(buf,",");
		}
		wtk_strbuf_push_f(buf,"at=\"");
		wtk_crfact_value_queue_print(v->at,buf);
		wtk_strbuf_push_f(buf,"\"");
		pad=1;
	}
	if(v->t)
	{
		if(pad)
		{
			wtk_strbuf_push_f(buf,",");
		}
		wtk_strbuf_push_f(buf,"t=\"");
		wtk_crfact_value_print(v->t,buf);
		wtk_strbuf_push_f(buf,"\"");
		pad=1;
	}
	if(v->is)
	{
		if(pad)
		{
			wtk_strbuf_push_f(buf,",");
		}
		wtk_strbuf_push_f(buf,"is=\"");
		wtk_crfact_value_print(v->is,buf);
		wtk_strbuf_push_f(buf,"\"");
		pad=1;
	}
	if(v->rt)
	{
		if(pad)
		{
			wtk_strbuf_push_f(buf,",");
		}
		wtk_strbuf_push_f(buf,"rt=\"");
		wtk_crfact_value_print(v->rt,buf);
		wtk_strbuf_push_f(buf,"\"");
		pad=1;
	}
	if(v->st)
	{
		if(pad)
		{
			wtk_strbuf_push_f(buf,",");
		}
		wtk_strbuf_push_f(buf,"st=\"");
		wtk_crfact_value_queue_print(v->st,buf);
		wtk_strbuf_push_f(buf,"\"");
		pad=1;
	}
	if(v->apn)
	{
		if(pad)
		{
			wtk_strbuf_push_f(buf,",");
		}
		wtk_strbuf_push_f(buf,"apn=\"");
		wtk_crfact_value_print(v->apn,buf);
		wtk_strbuf_push_f(buf,"\"");
		pad=1;
	}
	if(v->pn)
	{
		if(pad)
		{
			wtk_strbuf_push_f(buf,",");
		}
		wtk_strbuf_push_f(buf,"pn=\"");
		wtk_crfact_value_print(v->pn,buf);
		wtk_strbuf_push_f(buf,"\"");
		pad=1;
	}
	//wtk_debug("pa=%p\n",v->pa);
	if(v->pa)
	{
		if(pad)
		{
			wtk_strbuf_push_f(buf,",");
		}
		wtk_strbuf_push_f(buf,"pa=\"");
		wtk_crfact_value_print(v->pa,buf);
		wtk_strbuf_push_f(buf,"\"");
		pad=1;
	}
	if(v->pvd)
	{
		if(pad)
		{
			wtk_strbuf_push_f(buf,",");
		}
		wtk_strbuf_push_f(buf,"pvd=\"");
		wtk_crfact_value_queue_print(v->pvd,buf);
		wtk_strbuf_push_f(buf,"\"");
		pad=1;
	}
	if(v->pv1)
	{
		if(pad)
		{
			wtk_strbuf_push_f(buf,",");
		}
		wtk_strbuf_push_f(buf,"pv1=\"");
		wtk_crfact_value_queue_print(v->pv1,buf);
		wtk_strbuf_push_f(buf,"\"");
		pad=1;
	}
	if(v->pv)
	{
		if(pad)
		{
			wtk_strbuf_push_f(buf,",");
		}
		wtk_strbuf_push_f(buf,"pv=\"");
		wtk_crfact_value_print(v->pv,buf);
		wtk_strbuf_push_f(buf,"\"");
		pad=1;
	}
	wtk_strbuf_push_f(buf,")");
	//printf("\n");
}


void wtk_crfact_print(wtk_crfact_t *act)
{
	wtk_queue_node_t *qn;
	wtk_crfact_item_t *v;
	wtk_strbuf_t *buf;

	buf=wtk_strbuf_new(256,1);
	for(qn=act->act_item_q.pop;qn;qn=qn->next)
	{
		v=data_offset2(qn,wtk_crfact_item_t,q_n);
		wtk_crfact_item_print2(v,buf);
		printf("%.*s\n",buf->pos,buf->data);
	}
	wtk_strbuf_delete(buf);
}

void wtk_crfact_item_get_full_value(wtk_crfact_item_t *item,char *k,int k_bytes,wtk_strbuf_t *buf)
{
	wtk_strbuf_reset(buf);
	if(wtk_str_equal_s(k,k_bytes,"ap"))
	{
		if(item->ap)
		{
			wtk_crfact_value_queue_print(item->ap,buf);
		}
	}else if(wtk_str_equal_s(k,k_bytes,"p"))
	{
		if(item->p)
		{
			wtk_crfact_value_print(item->p,buf);
		}
	}else if(wtk_str_equal_s(k,k_bytes,"vtd"))
	{
		if(item->vtd)
		{
			wtk_crfact_value_queue_print(item->vtd,buf);
		}
	}else if(wtk_str_equal_s(k,k_bytes,"vt1"))
	{
		if(item->vt1)
		{
			wtk_crfact_value_queue_print(item->vt1,buf);
		}
	}else if(wtk_str_equal_s(k,k_bytes,"vt"))
	{
		if(item->vt)
		{
			wtk_crfact_value_print(item->vt,buf);
		}
	}else if(wtk_str_equal_s(k,k_bytes,"at"))
	{
		if(item->at)
		{
			wtk_crfact_value_queue_print(item->at,buf);
		}
	}else if(wtk_str_equal_s(k,k_bytes,"st"))
	{
		if(item->st)
		{
			wtk_crfact_value_queue_print(item->st,buf);
		}
	}else if(wtk_str_equal_s(k,k_bytes,"t"))
	{
		if(item->t)
		{
			wtk_crfact_value_print(item->t,buf);
		}
	}else if(wtk_str_equal_s(k,k_bytes,"apn"))
	{
		if(item->apn)
		{
			wtk_crfact_value_print(item->apn,buf);
		}
	}else if(wtk_str_equal_s(k,k_bytes,"pn"))
	{
		if(item->pn)
		{
			wtk_crfact_value_print(item->pn,buf);
		}
	}else if(wtk_str_equal_s(k,k_bytes,"pa"))
	{
		if(item->pa)
		{
			wtk_crfact_value_print(item->pa,buf);
		}
	}else if(wtk_str_equal_s(k,k_bytes,"pvd"))
	{
		if(item->pvd)
		{
			wtk_crfact_value_queue_print(item->pvd,buf);
		}
	}else if(wtk_str_equal_s(k,k_bytes,"pv1"))
	{
		if(item->pv1)
		{
			wtk_crfact_value_queue_print(item->pv1,buf);
		}
	}else if(wtk_str_equal_s(k,k_bytes,"pv"))
	{
		if(item->pv)
		{
			wtk_crfact_value_print(item->pv,buf);
		}
	}else if(wtk_str_equal_s(k,k_bytes,"rt"))
	{
		if(item->rt)
		{
			wtk_crfact_value_print(item->rt,buf);
		}
	}else if(wtk_str_equal_s(k,k_bytes,"is"))
	{
		if(item->is)
		{
			wtk_crfact_value_print(item->is,buf);
		}
	}
}

wtk_string_t* wtk_crfact_merge_queue_value(wtk_crfact_t *act,wtk_queue_t *q)
{
	//wtk_strbuf_t *buf;
	wtk_queue_node_t *qn;
	wtk_crfact_value_t *v;
	//wtk_string_t *vp;

	if(!q){return NULL;}
	//buf=wtk_strbuf_new(256,1);
	for(qn=q->pop;qn;qn=qn->next)
	{
		v=data_offset2(qn,wtk_crfact_value_t,q_n);
//		if(v->wrd)
//		{
//			wtk_strbuf_push(buf,v->wrd->data,v->wrd->len);
//		}
		if(v->wrd)
		{
			return v->wrd;
		}
	}
//	if(buf->pos>0)
//	{
//		vp=wtk_heap_dup_string(act->
//	}
//	wtk_strbuf_delete(buf);
	return NULL;
}

wtk_string_t* wtk_crfact_get_value(wtk_crfact_t *act,char *k,int k_bytes)
{
	wtk_crfact_item_t *item;

	//wtk_debug("get key [%.*s]\n",k_bytes,k);
	if(!act || !act->item){return NULL;}
	item=act->item;
	if(wtk_str_equal_s(k,k_bytes,"act"))
	{
		return item->type;
	}else if(wtk_str_equal_s(k,k_bytes,"p"))
	{
		return item->p?item->p->wrd:NULL;
	}else if(wtk_str_equal_s(k,k_bytes,"p.inst"))
	{
		return item->p?item->p->inst:NULL;
	}else if(wtk_str_equal_s(k,k_bytes,"p.class"))
	{
		return item->p?item->p->cls:NULL;
	}
	else if(wtk_str_equal_s(k,k_bytes,"vt"))
	{
		return item->vt?item->vt->wrd:NULL;
	}else if(wtk_str_equal_s(k,k_bytes,"vt.inst"))
	{
		return item->vt?item->vt->inst:NULL;
	}else if(wtk_str_equal_s(k,k_bytes,"t"))
	{
		return item->t?item->t->wrd:NULL;
	}else if(wtk_str_equal_s(k,k_bytes,"t.class"))
	{
		return item->t?item->t->cls:NULL;
	}else if(wtk_str_equal_s(k,k_bytes,"t.inst"))
	{
		return item->t?item->t->inst:NULL;
	}else if(wtk_str_equal_s(k,k_bytes,"rt"))
	{
		return item->rt?item->rt->wrd:NULL;
	}else if(wtk_str_equal_s(k,k_bytes,"rt.inst"))
	{
		return item->rt?item->rt->inst:NULL;
	}else if(wtk_str_equal_s(k,k_bytes,"is"))
	{
		return item->is?item->is->wrd:NULL;
	}else if(wtk_str_equal_s(k,k_bytes,"pa"))
	{
		return item->pa?item->pa->wrd:0;
	}else if(wtk_str_equal_s(k,k_bytes,"pv"))
	{
		return item->pv?item->pv->wrd:0;
	}else if(wtk_str_equal_s(k,k_bytes,"pv.inst"))
	{
		return item->pv?item->pv->inst:0;
	}else if(wtk_str_equal_s(k,k_bytes,"ap"))
	{
		return wtk_crfact_merge_queue_value(act,item->ap);
	}
	return NULL;
}

int wtk_crfact_item_has_key(wtk_crfact_item_t *item,char *k,int k_bytes)
{
	if(wtk_str_equal_s(k,k_bytes,"act"))
	{
		return item->type?1:0;
	}else if(wtk_str_equal_s(k,k_bytes,"ap"))
	{
		return item->ap?1:0;
	}else if(wtk_str_equal_s(k,k_bytes,"p"))
	{
		return item->p?1:0;
	}else if(wtk_str_equal_s(k,k_bytes,"p.inst"))
	{
		return item->p?(item->p->inst?1:0):0;
	}else if(wtk_str_equal_s(k,k_bytes,"p.class"))
	{
		return item->p?(item->p->cls?1:0):0;
	}else if(wtk_str_equal_s(k,k_bytes,"vtd"))
	{
		return item->vtd?1:0;
	}else if(wtk_str_equal_s(k,k_bytes,"vt1"))
	{
		return item->vt1?1:0;
	}else if(wtk_str_equal_s(k,k_bytes,"vt"))
	{
		return item->vt?1:0;
	}else if(wtk_str_equal_s(k,k_bytes,"vt.inst"))
	{
		return item->vt?(item->vt->inst?1:0):0;
	}else if(wtk_str_equal_s(k,k_bytes,"vt.class"))
	{
		return item->vt?(item->vt->cls?1:0):0;
	}else if(wtk_str_equal_s(k,k_bytes,"t"))
	{
		return item->t?1:0;
	}else if(wtk_str_equal_s(k,k_bytes,"t.inst"))
	{
		return item->t?(item->t->inst?1:0):0;
	}else if(wtk_str_equal_s(k,k_bytes,"t.class"))
	{
		return item->t?(item->t->cls?1:0):0;
	}else if(wtk_str_equal_s(k,k_bytes,"at"))
	{
		return item->at?1:0;
	}else if(wtk_str_equal_s(k,k_bytes,"st"))
	{
		return item->st?1:0;
	}else if(wtk_str_equal_s(k,k_bytes,"apn"))
	{
		return item->apn?1:0;
	}else if(wtk_str_equal_s(k,k_bytes,"pn"))
	{
		return item->pn?1:0;
	}else if(wtk_str_equal_s(k,k_bytes,"pa"))
	{
		return item->pa?1:0;
	}else if(wtk_str_equal_s(k,k_bytes,"pv"))
	{
		return item->pv?1:0;
	}else if(wtk_str_equal_s(k,k_bytes,"pvd"))
	{
		return item->pvd?1:0;
	}else if(wtk_str_equal_s(k,k_bytes,"pv1"))
	{
		return item->pv1?1:0;
	}else if(wtk_str_equal_s(k,k_bytes,"rt"))
	{
		return item->rt?1:0;
	}else if(wtk_str_equal_s(k,k_bytes,"is"))
	{
		return item->is?1:0;
	}
	return 0;
}

int wtk_crfact_item_nvalue(wtk_crfact_item_t *item)
{
	int n;

	n=0;
	n+=item->type?1:0;
	if(item->ap)
	{
		n+=1;
	}
	if(item->p)
	{
		n+=1;
	}
	if(item->vtd && item->vtd->length>0)
	{
		n+=1;
	}
	if(item->vt1 && item->vt1->length>0)
	{
		n+=1;
	}
	if(item->vt)
	{
		n+=1;
	}
	if(item->at && item->at->length>0)
	{
		n+=1;
	}
	if(item->st && item->st->length>0)
	{
		n+=1;
	}
	if(item->t)
	{
		n+=1;
	}
	if(item->rt)
	{
		n+=1;
	}
	if(item->is)
	{
		n+=1;
	}
	if(item->apn)
	{
		n+=1;
	}
	if(item->pn)
	{
		n+=1;
	}
	if(item->pa)
	{
		n+=1;
	}
	if(item->pvd && item->pvd->length>0)
	{
		n+=1;
	}
	if(item->pv1 && item->pv1->length>0)
	{
		n+=1;
	}
	if(item->pv)
	{
		n+=1;
	}
	//wtk_debug("nvalue=%d\n",n);
	//exit(0);
	return n;
}


typedef enum
{
	WTK_CRFACT_INIT,
	WTK_CRFACT_AP,
	WTK_CRFACT_P,
	WTK_CRFACT_P_END,
	WTK_CRFACT_VT,
	WTK_CRFACT_APN,
	WTK_CRFACT_T,
}wtk_crfact_state_t;

/**
 *
 * * question: .ask|.check|.for|.how|.is_or_not|.select|.where|.who|.why
 * * ap|apn|at|n|p|pa|pn|pv|pv1|pvd|t|v|vt|vt1|vtd
 *
 * example:
.for p p pv 因为 兔子 肉 好吃 啊
.for p pv 因为 我 淋雨 了
.for p vt t 因为 他 是 肉 啊
.for p vt1 vt t t 因为 啊 爸爸 喜欢 吃 兔子 肉
.why p p pv 为什么 兔子 肉 好吃 呢
.why p vt t 为什么 爸爸 喜欢 兔子
.why p vt1 vt t t 为什么 爸爸 喜欢 吃 兔子 肉 呢
.why p vtd vt t 我 为什么 你 不 喜欢 爸爸
ap p .where 爸爸 的 老家 在 什么 地方 你 知道 吗
ap p .who 柯南 的 爸爸 是 谁
p .how pa 地球 有 多 大
p .is_or_not t 你 中不中意 我
p .where 薄熙来 现在 在 哪里
p .who 小明 是 谁
p .why pvd pv 你 怎么 还 不 死 啊
p .why pvd pv1 pv 你 怎么 还 不 去 死
p apn pn 地球 上 最大 的 海洋
p apn pn .ask 世界 上 最长 的 河 是 哪条 河
p p .where 大力 水手 在 什么 地方
p p vt t 大力 水手 喜欢 爸爸
p pa 今天 天气 很 好
p pa .check 你 不 觉得 兔子 可爱 吗
p pv 那 你 就 好好 休息 吧
p pv .check 你 休息 了 吗
p t vt 我 觉得 兔子 胖嘟嘟 的 还是 蛮 喜欢 的 啊
p v at t 我 不是 你 爸爸
p vt t 猫 长 的 像 老虎
p vt t .select t 你 是 帅哥 还是 美女 呀
p vt t t 爸爸 喜欢 大力 水手
p vt1 vt1 vt t 我 要 去 看 医生
p vtd vt t 我 不会 想念 你

section:
*start: [.ask|.check|.for|.how|.is_or_not|.select|.where|.who|.why|p]
* ap* p+ (vt1|vtd)* vt at* t+
* ap* p+ apn pn   地球 上 最大 的 海洋
* ap* p+ pa  今天 天气 很 好
* ap* p+ pv 那 你 就 好好 休息 吧
* ap* p+ t (vt1|vtd)* vt 我 觉得 兔子 胖嘟嘟 的 还是 蛮 喜欢 的 啊
*
* ap* p+ (vt1|vtd|vt|apn|pa|pv|t)
 */
int wtk_crfact_parser_feed_act(wtk_crfact_parser_t *p,wtk_crfact_t *act,
		wtk_crfact_state_t *state,wtk_string_t *wrd,wtk_string_t *pos,const char *crf)
{
	wtk_heap_t *heap=p->heap;
	wtk_crfact_value_t *value;
	int ret=-1;
	wtk_strbuf_t *buf=p->buf;
	wtk_string_t t;

	switch(*state)
	{
	case WTK_CRFACT_INIT:
		if(strcmp(crf,"n")==0)
		{
			ret=0;
			goto end;
		}else if(strcmp(crf,"ap")==0)
		{
			act->item=wtk_crfact_item_new(heap);
			wtk_queue_push(&(act->act_item_q),&(act->item->q_n));
			act->item->ap=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
			wtk_queue_init(act->item->ap);
			value=wtk_crfact_value_new(heap,wrd,pos);
			wtk_queue_push(act->item->ap,&(value->q_n));
			*state=WTK_CRFACT_AP;
		}else if(strcmp(crf,"p")==0)
		{
			act->item=wtk_crfact_item_new(heap);
			wtk_queue_push(&(act->act_item_q),&(act->item->q_n));
			act->item->p=wtk_crfact_value_new(heap,wrd,pos);
			*state=WTK_CRFACT_P;
		}else if(strcmp(crf,"pa")==0)
		{
			act->item=wtk_crfact_item_new(heap);
			wtk_queue_push(&(act->act_item_q),&(act->item->q_n));
			act->item->pa=wtk_crfact_value_new(heap,wrd,pos);
			//wtk_debug("set pa=%p\n",act->item->pa);
		}else if(crf[0]=='.')
		{
			if(!act->item || act->item->type)
			{
				act->item=wtk_crfact_item_new(heap);
				wtk_queue_push(&(act->act_item_q),&(act->item->q_n));
				act->item->type_value=wtk_crfact_value_new(heap,wrd,pos);
				act->item->type=wtk_heap_dup_string(heap,(char*)(crf)+1,strlen(crf)-1);
				*state=WTK_CRFACT_AP;
			}else
			{
				act->item->type_value=wtk_crfact_value_new(heap,wrd,pos);
				act->item->type=wtk_heap_dup_string(heap,(char*)(crf)+1,strlen(crf)-1);
			}
		}else
		{
			//wtk_debug("unk [%.*s][%.*s]=%s\n",wrd->len,wrd->data,pos->len,pos->data,crf);
			//exit(0);
			ret=0;
			goto end;
		}
		break;
	case WTK_CRFACT_AP:
		if(strcmp(crf,"n")==0)
		{
			ret=0;
			goto end;
		}else if(strcmp(crf,"ap")==0)
		{
			value=wtk_crfact_value_new(heap,wrd,pos);
			wtk_queue_push(act->item->ap,&(value->q_n));
		}else if(strcmp(crf,"p")==0)
		{
			act->item->p=wtk_crfact_value_new(heap,wrd,pos);
			*state=WTK_CRFACT_P;
		}else if(crf[0]=='.')
		{
			act->item->type_value=wtk_crfact_value_new(heap,wrd,pos);
			act->item->type=wtk_heap_dup_string(heap,(char*)(crf)+1,strlen(crf)-1);
		}else
		{
			wtk_debug("unk [%.*s][%.*s]=%s\n",wrd->len,wrd->data,pos->len,pos->data,crf);
			goto end;
		}
		break;
	case WTK_CRFACT_P:
		if(strcmp(crf,"p")==0)
		{
			wtk_strbuf_reset(buf);
			wtk_strbuf_push(buf,act->item->p->wrd->data,act->item->p->wrd->len);
			wtk_strbuf_push_s(buf," ");
			wtk_strbuf_push(buf,wrd->data,wrd->len);
			wtk_string_set(&(t),buf->data,buf->pos);
			act->item->p=wtk_crfact_value_new(heap,&t,pos);
		}else
		{
			*state=WTK_CRFACT_P_END;
			return wtk_crfact_parser_feed_act(p,act,state,wrd,pos,crf);
		}
		break;
	case WTK_CRFACT_P_END:
		//(vt1|vtd|vt|apn|pa|pv|t)
		if(strcmp(crf,"n")==0)
		{
			ret=0;
			goto end;
		}else if(strcmp(crf,"vt1")==0)
		{
			if(!act->item->vt1)
			{
				act->item->vt1=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
				wtk_queue_init(act->item->vt1);
			}
			value=wtk_crfact_value_new(heap,wrd,pos);
			wtk_queue_push(act->item->vt1,&(value->q_n));
		}else if(strcmp(crf,"vtd")==0)
		{
			if(!act->item->vtd)
			{
				act->item->vtd=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
				wtk_queue_init(act->item->vtd);
			}
			value=wtk_crfact_value_new(heap,wrd,pos);
			wtk_queue_push(act->item->vtd,&(value->q_n));
		}else if(strcmp(crf,"vt")==0)
		{
			act->item->vt=wtk_crfact_value_new(heap,wrd,pos);
			*state=WTK_CRFACT_VT;
		}else if(strcmp(crf,"apn")==0)
		{
			act->item->apn=wtk_crfact_value_new(heap,wrd,pos);
			*state=WTK_CRFACT_APN;
		}else if(strcmp(crf,"pa")==0)
		{
			act->item->pa=wtk_crfact_value_new(heap,wrd,pos);
			//wtk_debug("set pa=%p\n",act->item->pa);
			*state=WTK_CRFACT_INIT;
		}else if(strcmp(crf,"pvd")==0)
		{
			if(!act->item->pvd)
			{
				act->item->pvd=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
				wtk_queue_init(act->item->pvd);
			}
			value=wtk_crfact_value_new(heap,wrd,pos);
			wtk_queue_push(act->item->pvd,&(value->q_n));
		}else if(strcmp(crf,"pv1")==0)
		{
			if(!act->item->pv1)
			{
				act->item->pv1=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
				wtk_queue_init(act->item->pv1);
			}
			value=wtk_crfact_value_new(heap,wrd,pos);
			wtk_queue_push(act->item->pv1,&(value->q_n));
		}else if(strcmp(crf,"pv")==0)
		{
			act->item->pv=wtk_crfact_value_new(heap,wrd,pos);
			*state=WTK_CRFACT_INIT;
		}else if(strcmp(crf,"t")==0)
		{
			act->item->t=wtk_crfact_value_new(heap,wrd,pos);
			*state=WTK_CRFACT_T;
		}else if(strcmp(crf,"st")==0)
		{
			if(!act->item->st)
			{
				act->item->st=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
				wtk_queue_init(act->item->st);
			}
			value=wtk_crfact_value_new(heap,wrd,pos);
			wtk_queue_push(act->item->st,&(value->q_n));
		}else if(strcmp(crf,"at")==0)
		{
			if(!act->item->at)
			{
				act->item->at=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
				wtk_queue_init(act->item->at);
			}
			value=wtk_crfact_value_new(heap,wrd,pos);
			wtk_queue_push(act->item->at,&(value->q_n));
		}else if(strcmp(crf,"rt")==0)
		{
			if(act->item->rt)
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push(buf,act->item->rt->wrd->data,act->item->rt->wrd->len);
				wtk_strbuf_push_s(buf," ");
				wtk_strbuf_push(buf,wrd->data,wrd->len);
				wtk_string_set(&(t),buf->data,buf->pos);
				act->item->rt=wtk_crfact_value_new(heap,&t,pos);
			}else
			{
				act->item->rt=wtk_crfact_value_new(heap,wrd,pos);
			}
		}else if(strcmp(crf,"is")==0)
		{
			act->item->is=wtk_crfact_value_new(heap,wrd,pos);
		}
		else if(crf[0]=='.')
		{
			act->item->type_value=wtk_crfact_value_new(heap,wrd,pos);
			act->item->type=wtk_heap_dup_string(heap,(char*)(crf)+1,strlen(crf)-1);
		}else
		{
			*state=WTK_CRFACT_INIT;
			return wtk_crfact_parser_feed_act(p,act,state,wrd,pos,crf);
			//wtk_debug("unk [%.*s][%.*s]=%s\n",wrd->len,wrd->data,pos->len,pos->data,crf);
			goto end;
		}
		break;
	case WTK_CRFACT_APN:
		if(strcmp(crf,"n")==0)
		{
			ret=0;
			goto end;
		}else if(strcmp(crf,"pn")==0)
		{
			act->item->pn=wtk_crfact_value_new(heap,wrd,pos);
			*state=WTK_CRFACT_INIT;
		}
		else if(crf[0]=='.')
		{
			act->item->type_value=wtk_crfact_value_new(heap,wrd,pos);
			act->item->type=wtk_heap_dup_string(heap,(char*)(crf)+1,strlen(crf)-1);
		}else
		{
			*state=WTK_CRFACT_INIT;
			return wtk_crfact_parser_feed_act(p,act,state,wrd,pos,crf);
			//wtk_debug("unk [%.*s][%.*s]=%s\n",wrd->len,wrd->data,pos->len,pos->data,crf);
			goto end;
		}
		break;
	case WTK_CRFACT_VT:
		if(strcmp(crf,"n")==0)
		{
			ret=0;
			goto end;
		}else if(strcmp(crf,"at")==0)
		{
			if(!act->item->at)
			{
				act->item->at=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
				wtk_queue_init(act->item->at);
			}
			value=wtk_crfact_value_new(heap,wrd,pos);
			wtk_queue_push(act->item->at,&(value->q_n));
		}
		else if(strcmp(crf,"t")==0)
		{
			act->item->t=wtk_crfact_value_new(heap,wrd,pos);
			*state=WTK_CRFACT_T;
		}
		else if(crf[0]=='.')
		{
//			act->item->type_value=wtk_crfact_value_new(heap,wrd,pos);
//			act->item->type=wtk_heap_dup_string(heap,(char*)(crf)+1,strlen(crf)-1);
			if(act->item->type)
			{
				if(wtk_str_equal_s(crf+1,strlen(crf)-1,"select")&&wtk_string_cmp_s(act->item->type,"select"))
				{

				}else
				{
					act->item=wtk_crfact_item_new(heap);
					wtk_queue_push(&(act->act_item_q),&(act->item->q_n));
					act->item->type_value=wtk_crfact_value_new(heap,wrd,pos);
					act->item->type=wtk_heap_dup_string(heap,(char*)(crf)+1,strlen(crf)-1);
					*state=WTK_CRFACT_AP;
				}
			}else
			{
				act->item->type_value=wtk_crfact_value_new(heap,wrd,pos);
				act->item->type=wtk_heap_dup_string(heap,(char*)(crf)+1,strlen(crf)-1);
			}
		}else
		{
			*state=WTK_CRFACT_INIT;
			return wtk_crfact_parser_feed_act(p,act,state,wrd,pos,crf);
			//wtk_debug("unk [%.*s][%.*s]=%s\n",wrd->len,wrd->data,pos->len,pos->data,crf);
			goto end;
		}
		break;
	case WTK_CRFACT_T:
		if(strcmp(crf,"n")==0)
		{
			ret=0;
			goto end;
		}else if(strcmp(crf,"t")==0)
		{
			wtk_strbuf_reset(buf);
			wtk_strbuf_push(buf,act->item->t->wrd->data,act->item->t->wrd->len);
			wtk_strbuf_push_s(buf," ");
			wtk_strbuf_push(buf,wrd->data,wrd->len);
			wtk_string_set(&(t),buf->data,buf->pos);
			act->item->t=wtk_crfact_value_new(heap,&t,pos);
			*state=WTK_CRFACT_T;
		}else if(strcmp(crf,"rt")==0)
		{
			if(act->item->rt)
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push(buf,act->item->rt->wrd->data,act->item->rt->wrd->len);
				wtk_strbuf_push_s(buf," ");
				wtk_strbuf_push(buf,wrd->data,wrd->len);
				wtk_string_set(&(t),buf->data,buf->pos);
				act->item->rt=wtk_crfact_value_new(heap,&t,pos);
			}else
			{
				act->item->rt=wtk_crfact_value_new(heap,wrd,pos);
			}
		}else if(strcmp(crf,"is")==0)
		{
			act->item->is=wtk_crfact_value_new(heap,wrd,pos);
		}else if(strcmp(crf,"vt1")==0)
		{
			if(!act->item->vt1)
			{
				act->item->vt1=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
				wtk_queue_init(act->item->vt1);
			}
			value=wtk_crfact_value_new(heap,wrd,pos);
			wtk_queue_push(act->item->vt1,&(value->q_n));
		}else if(strcmp(crf,"vtd")==0)
		{
			if(!act->item->vtd)
			{
				act->item->vtd=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
				wtk_queue_init(act->item->vtd);
			}
			value=wtk_crfact_value_new(heap,wrd,pos);
			wtk_queue_push(act->item->vtd,&(value->q_n));
		}else if(strcmp(crf,"vt")==0)
		{
			act->item->vt=wtk_crfact_value_new(heap,wrd,pos);
			*state=WTK_CRFACT_VT;
		}
		else if(crf[0]=='.')
		{
			act->item->type_value=wtk_crfact_value_new(heap,wrd,pos);
			act->item->type=wtk_heap_dup_string(heap,(char*)(crf)+1,strlen(crf)-1);
		}else if(strcmp(crf,"st")==0)
		{
			if(!act->item->st)
			{
				act->item->st=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
				wtk_queue_init(act->item->st);
			}
			value=wtk_crfact_value_new(heap,wrd,pos);
			wtk_queue_push(act->item->st,&(value->q_n));
		}else
		{
			*state=WTK_CRFACT_INIT;
			return wtk_crfact_parser_feed_act(p,act,state,wrd,pos,crf);
			//wtk_debug("unk [%.*s][%.*s]=%s\n",wrd->len,wrd->data,pos->len,pos->data,crf);
			goto end;
		}
		break;
	}
	ret=0;
end:
	return ret;
}

wtk_crfact_t* wtk_crfact_parser_process(wtk_crfact_parser_t *p,char *s,int len)
{
	static wtk_string_t xi=wtk_string("i");
	wtk_strbuf_t *buf=p->buf;
	wtk_poseg_t *seg=p->seg;
	wtk_crf_t *crf=p->crf;
	int i,n;
	const char *ts;
	wtk_crfact_t *act=NULL;
	wtk_heap_t *heap=p->heap;
	//wtk_crfact_item_t *item=NULL;
	//wtk_crfact_value_t *value;
	int ret;
	wtk_crfact_state_t state;

	//wtk_debug("[%.*s]\n",len,s);
	wtk_poseg_process(seg,s,len);
	for(i=0;i<seg->nwrd;++i)
	{
		//wtk_debug("v[%d]=%.*s %.*s\n",i,seg->wrds[i]->len,seg->wrds[i]->data,seg->pos[i]->len,seg->pos[i]->data);
		wtk_strbuf_reset(buf);
		wtk_strbuf_push(buf,seg->wrds[i]->data,seg->wrds[i]->len);
		wtk_strbuf_push_s(buf," ");
		wtk_strbuf_push(buf,seg->pos[i]->data,seg->pos[i]->len);
		wtk_strbuf_push_c(buf,0);
		ret=wtk_crf_add(crf,buf->data);
		if(ret!=0){goto end;}
	}
	//exit(0);
	ret=wtk_crf_process(crf);
	if(ret!=0){goto end;}
	n=wtk_crf_nresult(crf);
	//wtk_debug("n=%d\n",n);
	act=wtk_crfact_new(heap);
	state=WTK_CRFACT_INIT;
	for(i=0;i<n;++i)
	{
		ts=wtk_crf_get(crf,i);
		//wtk_debug("v[%d]=%.*s %.*s %s\n",i,seg->wrds[i]->len,seg->wrds[i]->data,seg->pos[i]->len,seg->pos[i]->data,ts);
		//printf("%.*s %.*s %s f=%f\n",seg->wrds[i]->len,seg->wrds[i]->data,seg->pos[i]->len,seg->pos[i]->data,ts,wtk_crf_get_prob(crf,i));
		printf("%.*s %.*s %s\n",seg->wrds[i]->len,seg->wrds[i]->data,seg->pos[i]->len,seg->pos[i]->data,ts);
		ret=wtk_crfact_parser_feed_act(p,act,&state,seg->wrds[i],seg->pos[i],ts);
		if(ret!=0){goto end;}
	}
	if(act->act_item_q.length<=0)
	{
		act=NULL;
	}else
	{
		if(!act->item->type)
		{
			act->item->type=&xi;
		}
	}
	//wtk_debug("len=%d\n",act->act_item_q.length);
	//wtk_crfact_print(act);
end:
#ifdef DEBUG_PROB
	wtk_debug("f=%f\n",f);
#endif
	//exit(0);
	wtk_poseg_reset(seg);
	wtk_crf_reset(crf);
	return act;
}

void wtk_crfact_item_merge_vt(wtk_crfact_item_t *item,wtk_strbuf_t *buf)
{
	wtk_queue_node_t *qn;
	wtk_crfact_value_t *vx;

	wtk_strbuf_reset(buf);
	if(item->vtd)
	{
		for(qn=item->vtd->pop;qn;qn=qn->next)
		{
			vx=data_offset2(qn,wtk_crfact_value_t,q_n);
			wtk_strbuf_push(buf,vx->wrd->data,vx->wrd->len);
		}
	}
	if(item->vt1)
	{
		for(qn=item->vt1->pop;qn;qn=qn->next)
		{
			vx=data_offset2(qn,wtk_crfact_value_t,q_n);
			wtk_strbuf_push(buf,vx->wrd->data,vx->wrd->len);
		}
	}
	if(item->vt)
	{
		wtk_strbuf_push(buf,item->vt->wrd->data,item->vt->wrd->len);
	}
}
#endif
