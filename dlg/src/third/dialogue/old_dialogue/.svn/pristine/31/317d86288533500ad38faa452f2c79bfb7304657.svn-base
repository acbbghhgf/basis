#include <ctype.h>
#include "wtk_semfstc.h" 
int wtk_semfstc_feed(wtk_semfstc_t *c,wtk_string_t *v);
int wtk_semfstc_compile_file2(wtk_semfstc_t *c,char *fn);

wtk_semfstc_t* wtk_semfstc_new()
{
	wtk_semfstc_t *c;

	c=(wtk_semfstc_t*)wtk_malloc(sizeof(wtk_semfstc_t));
	c->heap=wtk_heap_new(4096);
	c->buf=wtk_strbuf_new(256,1);
	c->tmp=wtk_strbuf_new(256,1);
	c->pwd=NULL;
	c->a=wtk_larray_new(30,sizeof(void*));
	c->rbin=NULL;
	wtk_semfstc_reset(c);
	return c;
}

void wtk_semfstc_delete(wtk_semfstc_t *c)
{
	wtk_larray_delete(c->a);
	wtk_strbuf_delete(c->buf);
	wtk_strbuf_delete(c->tmp);
	wtk_heap_delete(c->heap);
	wtk_free(c);
}

void wtk_semfstc_set_state(wtk_semfstc_t *c,wtk_semfstc_state_t state)
{
	c->state=state;
	c->sub_state=-1;
}

void wtk_semfstc_reset(wtk_semfstc_t *c)
{
	wtk_semfstc_set_state(c,WTK_SEMFSTC_INIT);
	c->pwd=NULL;
	c->r_state=WTK_SEMFSTR_INIT;
	wtk_heap_reset(c->heap);
	wtk_strbuf_reset(c->buf);
	wtk_strbuf_reset(c->tmp);
}

int wtk_semfstc_feed_init(wtk_semfstc_t *c,wtk_string_t *v)
{
enum
{
	WTK_SEMFSTC_INIT_INIT=-1,
};
	char t;
	wtk_strbuf_t *buf=c->buf;

	switch(c->sub_state)
	{
	case WTK_SEMFSTC_INIT_INIT:
		if(v->len==1)
		{
			t=v->data[0];
			switch(t)
			{
			case '[':
				wtk_strbuf_reset(buf);
				//wtk_strbuf_push_s(buf,"[");
				wtk_semfstc_set_state(c,WTK_SEMFSTC_ATTR);
				break;
			case '#':
				wtk_strbuf_reset(buf);
				wtk_semfstc_set_state(c,WTK_SEMFSTC_COMMENT);
				break;
			}
		}
		break;
	}
	return 0;
}

int wtk_semfstc_feed_comment(wtk_semfstc_t *c,wtk_string_t *v)
{
enum //wtk_semfstc__state_t
{
	WTK_SEMFSTC_COMMENT_INIT=-1,
	WTK_SEMFSTC_COMMENT_WAIT_END,
};
	char t;
	wtk_strbuf_t *buf=c->buf;

	//wtk_debug("%d=[%.*s]\n",c->sub_state,v->len,v->data);
	switch(c->sub_state)
	{
	case WTK_SEMFSTC_COMMENT_INIT:
		if(v->len==1)
		{
			t=v->data[0];
			switch(t)
			{
			case '\n':
				wtk_semfstc_set_state(c,WTK_SEMFSTC_INIT);
				break;
			default:
				if(isspace(t))
				{
					//wtk_debug("[%.*s]\n",buf->pos,buf->data);
					if(wtk_str_equal_s(buf->data,buf->pos,"include"))
					{
						wtk_semfstc_set_state(c,WTK_SEMFSTC_INCLUDE);
					}else
					{
						c->sub_state=WTK_SEMFSTC_COMMENT_WAIT_END;
					}
				}
				break;
			}
			wtk_strbuf_push(buf,v->data,v->len);
		}
		break;
	case WTK_SEMFSTC_COMMENT_WAIT_END:
		if(v->len==1 && v->data[0]=='\n')
		{
			wtk_semfstc_set_state(c,WTK_SEMFSTC_INIT);
		}
		break;
	}

	return 0;
}


wtk_string_t wtk_semfstc_expand_string(wtk_semfstc_t *c,wtk_string_t *v)
{
	wtk_strbuf_t *buf=c->tmp;
	wtk_var_parse_t *var;
	char *s,*e;
	int n;
	int ret;
	wtk_string_t t;

	//wtk_debug("[%.*s]\n",v->len,v->data);
	s=v->data;e=s+v->len;
	wtk_strbuf_reset(buf);
	var=wtk_var_parse_new();
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		ret=wtk_var_parse2(var,s,n);
		if(ret==1)
		{
			//wtk_debug("[%.*s]\n",var->buf->pos,var->buf->data);
			if(wtk_str_equal_s(var->buf->data,var->buf->pos,"pwd"))
			{
				if(c->pwd)
				{
					wtk_strbuf_push(buf,c->pwd->data,c->pwd->len);
				}
			}
			wtk_var_parse_reset(var);
		}else if(var->sub_state==0)
		{
			wtk_strbuf_push(buf,s,n);
		}
		s+=n;
	}
	wtk_string_set(&(t),buf->data,buf->pos);
	wtk_var_parse_delete(var);
	//wtk_debug("[%.*s]=[%.*s]\n",v->len,v->data,buf->pos,buf->data);
	//exit(0);
	return t;
}

void wtk_semfstc_set_string_state(wtk_semfstc_t *c,wtk_semfstc_state_t state,int sub_state)
{
	c->str_bak_state=state;
	c->str_bak_sub_state=sub_state;
	wtk_semfstc_set_state(c,WTK_SEMFSTC_STRING);
}

int wtk_semfstc_feed_string(wtk_semfstc_t *c,wtk_string_t *v)
{
enum
{
	WTK_SEMFSTC_STRING_INIT=-1,
	WTK_SEMFSTC_STRING_X,
};
	wtk_string_parser_t *p=&(c->str_parser);
	int ret;

	//wtk_debug("[%.*s]\n",v->len,v->data);
	switch(c->sub_state)
	{
	case WTK_SEMFSTC_STRING_INIT:
		wtk_strbuf_reset(c->tmp);
		wtk_string_parser_init(p,c->buf);
		ret=wtk_string_parse(p,v->data,v->len);
		c->sub_state=WTK_SEMFSTC_STRING_X;
		break;
	case WTK_SEMFSTC_STRING_X:
		ret=wtk_string_parse(p,v->data,v->len);
		if(ret==1)
		{
			//wtk_debug("[%.*s]=[%.*s]\n",v->len,v->data,p->buf->pos,p->buf->data);
			wtk_string_set(v,p->buf->data,p->buf->pos);
			wtk_semfstc_expand_string(c,v);
			//wtk_debug("[%.*s]=[%.*s]\n",v->len,v->data,p->buf->pos,p->buf->data);
			c->state=c->str_bak_state;
			c->sub_state=c->str_bak_sub_state;
			wtk_string_set(v,c->tmp->data,c->tmp->pos);
			//wtk_debug("[%.*s]\n",v->len,v->data);
			return wtk_semfstc_feed(c,v);
		}
		break;
	}
	return 0;
}

int wtk_semfstc_feed_include(wtk_semfstc_t *c,wtk_string_t *v)
{
enum
{
	WTK_SEMFSTC_INCLUDE_INIT=-1,
	WTK_SEMFSTC_INCLUDE_STRING,
};

	switch(c->sub_state)
	{
	case WTK_SEMFSTC_INCLUDE_INIT:
		wtk_semfstc_set_string_state(c,c->state,WTK_SEMFSTC_INCLUDE_STRING);
		return wtk_semfstc_feed(c,v);
		break;
	case WTK_SEMFSTC_INCLUDE_STRING:
		wtk_semfstc_set_state(c,WTK_SEMFSTC_INIT);
		//wtk_debug("[%.*s]\n",v->len,v->data);
		{
			wtk_strbuf_t *buf=c->buf;

			wtk_strbuf_reset(buf);
			wtk_strbuf_push(buf,v->data,v->len);
			wtk_strbuf_push_c(buf,0);
			wtk_semfstc_compile_file2(c,buf->data);
		}
		break;
	}
	return 0;
}

int wtk_semfstc_set_lexlib(wtk_semfstc_t *c,wtk_string_t *v,wtk_strkv_parser_t *p)
{
	wtk_string_t t;
	int ret;

	t=wtk_semfstc_expand_string(c,v);
	ret=wtk_semfst_net_set_lexlib(c->net,t.data,t.len);
	if(ret==0)
	{
		wtk_semfstc_set_state(c,WTK_SEMFSTC_INIT);
	}
	return ret;
}

int wtk_semfstc_set_sceen(wtk_semfstc_t *c,wtk_string_t *v,wtk_strkv_parser_t *p)
{
	c->sceen=wtk_semfst_net_pop_sceen(c->net,v->data,v->len);
	c->r_state=WTK_SEMFSTR_SCEEN;
	wtk_semfstc_set_state(c,WTK_SEMFSTC_INIT);
	return 0;
}

int wtk_semfstc_set_script(wtk_semfstc_t *c,wtk_string_t *v,wtk_strkv_parser_t *p)
{
	if(c->r_state!=WTK_SEMFSTR_SCEEN && c->r_state!=WTK_SEMFSTR_SCEEN_SCRIPT_OUTPUT)
	{
		wtk_debug("state not roight [%d]\n",c->r_state);
		return -1;
	}
	c->script=wtk_semfst_net_find_script(c->net,c->sceen,v->data,v->len,1);
	c->r_state=WTK_SEMFSTR_SCEEN_SCRIPT;
	wtk_semfstc_set_state(c,WTK_SEMFSTC_INIT);
	return 0;
}


int wtk_semfstc_set_lex(wtk_semfstc_t *c,wtk_string_t *v,wtk_strkv_parser_t *p)
{
	if(c->r_state==WTK_SEMFSTR_SCEEN_SCRIPT_OUTPUT || c->r_state==WTK_SEMFSTR_INIT)
	{
		c->sceen=wtk_semfst_net_pop_sceen(c->net,v->data,v->len);
		c->script=wtk_semfst_net_find_script(c->net,c->sceen,0,0,1);
		c->r_state=WTK_SEMFSTR_SCEEN_SCRIPT_LEX;
		wtk_semfstc_set_state(c,WTK_SEMFSTC_LEX);
		return 0;
	}
	if(c->r_state!=WTK_SEMFSTR_SCEEN_SCRIPT)
	{
		wtk_debug("state not roight [%d]\n",c->r_state);
		return -1;
	}
//	if(v)
//	{
//		wtk_debug("v[%.*s]\n",v->len,v->data);
//	}
	c->r_state=WTK_SEMFSTR_SCEEN_SCRIPT_LEX;
	wtk_semfstc_set_state(c,WTK_SEMFSTC_LEX);
	return 0;
}

int wtk_semfstc_feed_lex(wtk_semfstc_t *c,wtk_string_t *v)
{
enum
{
	WTK_SEMFSTC_LEX_INIT=-1,
	WTK_SEMFSTC_LEX_WAIT_BRACE1,
	WTK_SEMFSTC_LEX_WAIT_BRACE2,
	WTK_SEMFSTC_LEX_WAIT,
	WTK_SEMFSTC_LEX_WAIT_END1,
	WTK_SEMFSTC_LEX_WAIT_END2,
};
	wtk_strbuf_t *buf=c->buf;
	int ret;

	switch(c->sub_state)
	{
	case WTK_SEMFSTC_LEX_INIT:
		if(v->len==1 && v->data[0]=='{')
		{
			c->sub_state=WTK_SEMFSTC_LEX_WAIT_BRACE1;
		}
		break;
	case WTK_SEMFSTC_LEX_WAIT_BRACE1:
		if(v->len==1 && v->data[0]=='{')
		{
			c->sub_state=WTK_SEMFSTC_LEX_WAIT_BRACE2;
		}else
		{
			c->sub_state=WTK_SEMFSTC_LEX_INIT;
		}
		break;
	case WTK_SEMFSTC_LEX_WAIT_BRACE2:
		if(v->len==1 && v->data[0]=='{')
		{
			c->sub_state=WTK_SEMFSTC_LEX_WAIT;
			wtk_strbuf_reset(buf);
		}else
		{
			c->sub_state=WTK_SEMFSTC_LEX_INIT;
		}
		break;
	case WTK_SEMFSTC_LEX_WAIT:
		if(v->len==1 && v->data[0]=='}')
		{
			c->sub_state=WTK_SEMFSTC_LEX_WAIT_END1;
		}else
		{
			wtk_strbuf_push(buf,v->data,v->len);
		}
		break;
	case WTK_SEMFSTC_LEX_WAIT_END1:
		if(v->len==1 && v->data[0]=='}')
		{
			c->sub_state=WTK_SEMFSTC_LEX_WAIT_END2;
		}else
		{
			wtk_strbuf_push_s(buf,"}");
			wtk_strbuf_push(buf,v->data,v->len);
			c->sub_state=WTK_SEMFSTC_LEX_WAIT;
		}
		break;
	case WTK_SEMFSTC_LEX_WAIT_END2:
		if(v->len==1 && v->data[0]=='}')
		{
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			ret=wtk_semfst_net_set_script_lex(c->net,c->script,buf->data,buf->pos);
			if(ret!=0)
			{
				return ret;
			}
			c->r_state=WTK_SEMFSTR_SCEEN_SCRIPT;
			wtk_semfstc_set_state(c,WTK_SEMFSTC_INIT);
		}else
		{
			wtk_strbuf_push_s(buf,"}}");
			wtk_strbuf_push(buf,v->data,v->len);
			c->sub_state=WTK_SEMFSTC_LEX_WAIT;
		}
		break;
	}
	return 0;
}

int wtk_semfstc_set_output(wtk_semfstc_t *c,wtk_string_t *v,wtk_strkv_parser_t *p)
{
	int ret;
	int add=1;

	if(c->r_state!=WTK_SEMFSTR_SCEEN_SCRIPT && c->r_state!=WTK_SEMFSTR_SCEEN_SCRIPT_OUTPUT)
	{
		wtk_debug("state not roight [%d]\n",c->r_state);
		return -1;
	}
	c->output=wtk_semfst_net_pop_script_output(c->net,c->script);
	while(1)
	{
		ret=wtk_strkv_parser_next(p);
		if(ret!=0)
		{
			break;
		}
		//wtk_debug("[%.*s]=[%.*s]\n",p->k.len,p->k.data,p->v.len,p->v.data);
		if(wtk_string_cmp_s(&(p->k),"get_answer")==0)
		{
			c->output->get_answer=wtk_str_atoi(p->v.data,p->v.len);
		}else if(wtk_string_cmp_s(&(p->k),"reset")==0)
		{
			c->output->reset=wtk_str_atoi(p->v.data,p->v.len);
		}else if(wtk_string_cmp_s(&(p->k),"skip")==0)
		{
			c->output->skip=wtk_str_atoi(p->v.data,p->v.len);
		}else if(wtk_string_cmp_s(&(p->k),"other")==0)
		{
			c->script->other=c->output;
			add=0;
		}else if(wtk_string_cmp_s(&(p->k),"next")==0)
		{
			c->output->next=wtk_semfst_net_find_script(c->net,c->sceen,p->v.data,p->v.len,1);
		}
		else
		{
			wtk_semfst_net_set_script_output_slot(c->net,c->output,&(p->k),&(p->v));
		}
	}
	if(add)
	{
		wtk_queue3_push(&(c->script->output_q),&(c->output->q_n));
	}
	c->r_state=WTK_SEMFSTR_SCEEN_SCRIPT_OUTPUT;
	wtk_semfstc_set_state(c,WTK_SEMFSTC_OUTPUT);
	return 0;
}

int wtk_semfstc_feed_output(wtk_semfstc_t *c,wtk_string_t *v)
{
enum
{
	WTK_SEMFSTC_OUTPUT_INIT=-1,
	WTK_SEMFSTC_OUTPUT_WAIT_STR,
	WTK_SEMFSTC_OUTPUT_STR,
};
	wtk_strbuf_t *buf=c->buf;
	wtk_larray_t *a=c->a;
	wtk_semfst_output_item_t *si;

	//wtk_debug("[%.*s]\n",v->len,v->data);
	switch(c->sub_state)
	{
	case WTK_SEMFSTC_OUTPUT_INIT:
		wtk_larray_reset(a);
		if(v->len>1 || !isspace(v->data[0]))
		{
			c->sub_state=WTK_SEMFSTC_OUTPUT_WAIT_STR;
			return wtk_semfstc_feed_output(c,v);
		}
		return 0;
		break;
	case WTK_SEMFSTC_OUTPUT_WAIT_STR:
		if(v->len>1 || !isspace(v->data[0]))
		{
			if(v->data[0]=='[')
			{
				wtk_semfst_net_set_output_items(c->net,c->output,(wtk_semfst_output_item_t**)a->slot,a->nslot);
				//exit(0);
				//c->r_state=WTK_SEMFSTR_SCEEN_SCRIPT;
				wtk_semfstc_set_state(c,WTK_SEMFSTC_INIT);
				return wtk_semfstc_feed(c,v);
			}else
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push(buf,v->data,v->len);
				c->sub_state=WTK_SEMFSTC_OUTPUT_STR;
			}
		}else
		{
			//wtk_debug("====================\n");
			//exit(0);
			wtk_semfst_net_set_output_items(c->net,c->output,(wtk_semfst_output_item_t**)a->slot,a->nslot);
			//c->r_state=WTK_SEMFSTR_SCEEN_SCRIPT;
			wtk_semfstc_set_state(c,WTK_SEMFSTC_INIT);
			return wtk_semfstc_feed(c,v);
		}
		break;
	case WTK_SEMFSTC_OUTPUT_STR:
		if(v->len==1 && v->data[0]=='\n')
		{
			wtk_strbuf_strip(buf);
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			//wtk_semfstr_output_add_output(c->r,c->output,buf->data,buf->pos);
			si=wtk_semfst_net_output_new_output_item(c->net,buf->data,buf->pos);
			wtk_larray_push2(a,&(si));
			c->sub_state=WTK_SEMFSTC_OUTPUT_WAIT_STR;
		}else
		{
			wtk_strbuf_push(buf,v->data,v->len);
		}
		break;
	}
	return 0;
}


int wtk_semfstc_feed_attr(wtk_semfstc_t *c,wtk_string_t *v)
{
	wtk_strbuf_t *buf=c->buf;
	wtk_strkv_parser_t p;
	int ret;

	if(v->len==1 && v->data[0]==']')
	{
		wtk_strkv_parser_init(&(p),buf->data,buf->pos);
		while(1)
		{
			ret=wtk_strkv_parser_next(&(p));
			if(ret!=0)
			{
				break;
			}
			if(wtk_string_cmp_s(&(p.k),"lexlib")==0)
			{
				ret=wtk_semfstc_set_lexlib(c,&(p.v),&p);
			}else if(wtk_string_cmp_s(&(p.k),"sceen")==0)
			{
				ret=wtk_semfstc_set_sceen(c,&(p.v),&p);
			}else if(wtk_string_cmp_s(&(p.k),"script")==0)
			{
				ret=wtk_semfstc_set_script(c,&(p.v),&p);
			}else if(wtk_string_cmp_s(&(p.k),"lex")==0)
			{
				ret=wtk_semfstc_set_lex(c,&(p.v),&p);
			}else if(wtk_string_cmp_s(&(p.k),"output")==0)
			{
				ret=wtk_semfstc_set_output(c,&(p.v),&p);
			}
			else
			{
				wtk_debug("[%.*s]=[%.*s]\n",p.k.len,p.k.data,p.v.len,p.v.data);
				exit(0);
			}
			if(ret!=0)
			{
				wtk_debug("compile[%d] failed [%.*s]=[%.*s]\n",c->r_state,p.k.len,p.k.data,p.v.len,p.v.data);
			}
			return ret;
			break;
		}
		//exit(0);
	}else
	{
		wtk_strbuf_push(buf,v->data,v->len);
	}
	return 0;
}

int wtk_semfstc_feed(wtk_semfstc_t *c,wtk_string_t *v)
{
	int ret;

	ret=-1;
	switch(c->state)
	{
	case WTK_SEMFSTC_INIT:
		ret=wtk_semfstc_feed_init(c,v);
		break;
	case WTK_SEMFSTC_COMMENT:
		ret=wtk_semfstc_feed_comment(c,v);
		break;
	case WTK_SEMFSTC_INCLUDE:
		ret=wtk_semfstc_feed_include(c,v);
		break;
	case WTK_SEMFSTC_ATTR:
		ret=wtk_semfstc_feed_attr(c,v);
		break;
	case WTK_SEMFSTC_STRING:
		ret=wtk_semfstc_feed_string(c,v);
		break;
	case WTK_SEMFSTC_LEX:
		ret=wtk_semfstc_feed_lex(c,v);
		break;
	case WTK_SEMFSTC_OUTPUT:
		ret=wtk_semfstc_feed_output(c,v);
		break;
	}
	return ret;
}

int wtk_semfstc_compile_string(wtk_semfstc_t *c,char *data,int len)
{
	char *s,*e;
	int ret;
	int n;
	wtk_string_t v;

	s=data;e=s+len;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		wtk_string_set(&(v),s,n);
		ret=wtk_semfstc_feed(c,&v);
		if(ret!=0)
		{
			wtk_debug("compile failed\n");
			goto end;
		}
		s+=n;
	}
	ret=0;
end:
	return ret;
}

int wtk_semfstc_compile_file2(wtk_semfstc_t *c,char *fn)
{
	wtk_string_t *pwd;
	wtk_string_t v;
	int ret=-1;
	char *data=NULL;
	int len;
	wtk_rbin2_item_t *item=NULL;

	pwd=c->pwd;
	v=wtk_dir_name2(fn,strlen(fn),'/');
	c->pwd=wtk_heap_dup_string(c->heap,v.data,v.len);
	if(c->rbin)
	{
		item=wtk_rbin2_get3(c->rbin,fn,strlen(fn),0);
		if(!item){goto end;}
		data=item->data->data;
		len=item->data->len;
	}else
	{
		data=file_read_buf(fn,&len);
	}
	if(!data){goto end;}
	ret=wtk_semfstc_compile_string(c,data,len);
	if(ret!=0){goto end;}
	ret=0;
end:
	c->pwd=pwd;
	if(c->rbin)
	{
		if(item)
		{
			wtk_rbin2_item_clean(item);
		}
	}else
	{
		if(data)
		{
			wtk_free(data);
		}
	}
	return ret;
}

int wtk_semfstc_compile_file(wtk_semfstc_t *c,wtk_semfst_net_t *net,char *fn)
{
	c->net=net;
	wtk_semfstc_reset(c);
	return wtk_semfstc_compile_file2(c,fn);

}
