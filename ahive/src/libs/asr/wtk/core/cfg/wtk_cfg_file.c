#include "wtk_cfg_file.h"
#include "wtk/core/wtk_os.h"
#include <ctype.h>
int wtk_cfg_file_feed_expr_value_tok_end(wtk_cfg_file_t *cfg,char c);
int wtk_cfg_file_feed_expr_value_start(wtk_cfg_file_t *cfg,char c);
int wtk_cfg_file_feed_expr_value_tok_start(wtk_cfg_file_t *cfg,char c);
int wtk_cfg_file_feed_expr_tok_start(wtk_cfg_file_t* cfg,char c);
int wtk_cfg_file_feed_expr_start(wtk_cfg_file_t* cfg,char c);
int wtk_cfg_file_feed_string(wtk_cfg_file_t *c,char *d,int bytes);
wtk_cfg_if_item_t* wtk_cfg_file_get_last_if_item(wtk_cfg_file_t *cfg);
wtk_cfg_if_cond_t* wtk_cfg_file_get_last_if_cond(wtk_cfg_file_t *cfg);
wtk_cfg_if_cond_t* wtk_cfg_file_pop_item_cond(wtk_cfg_file_t *cfg);
void wtk_cfg_file_pop_condition(wtk_cfg_file_t *cfg);

int wtk_cfg_file_init(wtk_cfg_file_t *f)
{
	f->main=wtk_local_cfg_new_h(f->heap);
	f->cur=f->main;
	wtk_string_set_s(&(f->main->name),"main");
	wtk_queue_push(&(f->cfg_queue),&(f->main->q_n));
	f->scope=0;
	f->included=0;
	f->escaped=0;
	return 0;
}

wtk_cfg_file_t* wtk_cfg_file_new_fn(char *fn)
{
	return wtk_cfg_file_new_fn2(fn,1);
}

wtk_cfg_file_t* wtk_cfg_file_new_fn2(char *fn,int add_pwd)
{
	wtk_cfg_file_t *cfg=0;
	char *data=0;
	int len,ret;
	wtk_string_t *v=0;

	ret=0;
	data=file_read_buf(fn,&len);
	if(!data){goto end;}
	cfg=wtk_cfg_file_new();
	if(add_pwd)
	{
		v=wtk_dir_name(fn,'/');
		if(v->len>0)
		{
			wtk_cfg_file_add_var_ks(cfg,"pwd",v->data,v->len);
		}else
		{
			wtk_cfg_file_add_var_s_s(cfg,"pwd",".");
		}
	}
	ret=wtk_cfg_file_feed(cfg,data,len);
	if(v)
	{
		wtk_string_delete(v);
	}
end:
	if(data){free(data);}
	if(ret!=0)
	{
		wtk_cfg_file_delete(cfg);
		cfg=0;
	}
	return cfg;
}

wtk_cfg_file_t* wtk_cfg_file_new_fn3(wtk_string_t *dir,char *data,int len,int add_pwd)
{
	wtk_cfg_file_t *cfg=0;
	int ret;
//	wtk_string_t *v=0;

	ret=0;
	if(!data){goto end;}
	cfg=wtk_cfg_file_new();
	if(add_pwd)
	{
		wtk_cfg_file_add_var_ks(cfg,"pwd",dir->data,dir->len);
	}
	ret=wtk_cfg_file_feed(cfg,data,len);
end:
	if(ret!=0)
	{
		wtk_cfg_file_delete(cfg);
		cfg=0;
	}
	return cfg;
}

wtk_cfg_file_t *wtk_cfg_file_new()
{
	return wtk_cfg_file_new_ex(2048,4096);
}

wtk_cfg_file_t *wtk_cfg_file_new_ex(int buf_size,int heap_size)
{
	wtk_cfg_file_t *f;

	f=(wtk_cfg_file_t*)wtk_malloc(sizeof(*f));
	wtk_queue_init(&(f->cfg_queue));
	f->heap=wtk_heap_new(heap_size);
	f->tok=wtk_strbuf_new(buf_size,1);
	f->value=wtk_strbuf_new(buf_size,1);
	f->var=wtk_strbuf_new(buf_size,1);
	//f->comment=wtk_strbuf_new(buf_size,1);
	f->quoted=0;
	f->included=0;
	f->rbin=NULL;
	wtk_queue_init(&(f->if_q));
	wtk_cfg_file_init(f);
	return f;
}

int wtk_cfg_file_bytes2(wtk_cfg_file_t *cfg)
{
	return sizeof(*cfg)+wtk_strbuf_bytes(cfg->tok)+wtk_strbuf_bytes(cfg->value)+wtk_strbuf_bytes(cfg->var)+wtk_heap_bytes(cfg->heap);
}

int wtk_cfg_file_bytes(wtk_cfg_file_t *c)
{
	int bytes;

	bytes=sizeof(wtk_cfg_file_t);
	bytes+=wtk_heap_bytes(c->heap);
	bytes+=wtk_strbuf_bytes(c->tok);
	bytes+=wtk_strbuf_bytes(c->value);
	bytes+=wtk_strbuf_bytes(c->var);
	return bytes;
}

int wtk_cfg_file_reset(wtk_cfg_file_t *cfg)
{
	wtk_queue_init(&(cfg->cfg_queue));
	wtk_heap_reset(cfg->heap);
	cfg->quoted=0;
	cfg->included=0;
	return wtk_cfg_file_init(cfg);
}

int wtk_cfg_file_delete(wtk_cfg_file_t *c)
{
	wtk_heap_delete(c->heap);
	wtk_strbuf_delete(c->tok);
	wtk_strbuf_delete(c->value);
	wtk_strbuf_delete(c->var);
	//wtk_strbuf_delete(c->comment);
	wtk_free(c);
	return 0;
}

void wtk_cfg_file_set_rbin(wtk_cfg_file_t *cfile,wtk_rbin2_t *rbin)
{
	cfile->rbin=rbin;
}


int wtk_cfg_file_add_var(wtk_cfg_file_t *cfg,char *k,int kbytes,char *v,int vbytes)
{
	wtk_cfg_queue_add_string(cfg->main->cfg,k,kbytes,v,vbytes);
	return 0;
}

/*
	space=[ \r\t\n]
	digit=[0-9]
	alpha=[a-zA-Z]
	char=digit|alpha
	tok=	char+
	var=${tok}
	expr=	tok space*=space* value space*;
	value= 	tok | {space* expr* space*}
*/
#define is_value(c) (isalnum((c))||(c==':')||c=='/'||c=='\\'||c=='_'||c=='-'||c=='.')
#define is_char(c) (isalnum((c))||(c==':')||c=='_'||c=='.'||c=='-'||c=='/'||c=='@')

int wtk_cfg_file_process_include(wtk_cfg_file_t *cfg)
{
	int ret=-1;
	char *data;
	wtk_string_t *v;
	wtk_cfg_item_t *pth,*item;
	int n;
	wtk_cfg_queue_t *cq;
	wtk_rbin2_item_t *ritem=NULL;
	wtk_cfg_if_item_t* fi;

	fi=wtk_cfg_file_get_last_if_item(cfg);
	if(fi && fi->valid==0)
	{
		//wtk_debug("[%.*s]\n",cfg->value->pos,cfg->value->data);
		cfg->state=CF_EXPR_START;
		return 0;
	}
	wtk_strbuf_push_c(cfg->value,0);
	cfg->included=0;
	if(cfg->rbin)
	{
		ritem=wtk_rbin2_get3(cfg->rbin,cfg->value->data,strlen(cfg->value->data),0);
		if(ritem)
		{
			data=ritem->data->data;
			n=ritem->data->len;
		}else
		{
			data=NULL;
		}
	}else
	{
		data=file_read_buf(cfg->value->data,&n);
	}
	if(!data)
	{
		wtk_debug("%s not found.\n",cfg->value->data);
		goto end;
	}
	cfg->state=CF_EXPR_START;
	cq=cfg->cur->cfg;
	pth=wtk_cfg_queue_find_s(cq,"pwd");
	if(pth)
	{
		wtk_cfg_queue_remove(cq,pth);
	}
	v=wtk_dir_name(cfg->value->data,'/');
	if(!v){goto end;}
	wtk_cfg_queue_add_string(cq,"pwd",3,v->data,v->len);
	wtk_string_delete(v);
	ret=wtk_cfg_file_feed_string(cfg,data,n);
	if(cfg->rbin)
	{
		if(ritem)
		{
			wtk_rbin2_item_clean(ritem);
		}
	}else
	{
		wtk_free(data);
	}
	if(ret!=0){goto end;}
	item=wtk_cfg_queue_find_s(cq,"pwd");
	if(item)
	{
		wtk_cfg_queue_remove(cq,item);
	}
	if(pth)
	{
		wtk_cfg_queue_add(cq,pth);
	}
	cfg->state=CF_EXPR_START;
end:
	return ret;
}

int wtk_cfg_file_feed_expr_value_tok_end(wtk_cfg_file_t *cfg,char c)
{
	int ret=0;

	if(c==';')
	{
		//wtk_strbuf_push_c(cfg->value,0);
		if(cfg->included)
		{
			ret=wtk_cfg_file_process_include(cfg);
		}else
		{
			wtk_cfg_if_item_t* item;

			item=wtk_cfg_file_get_last_if_item(cfg);
			//wtk_debug("[%.*s]=[%.*s] item=%p/%d\n",cfg->tok->pos,cfg->tok->data,cfg->value->pos,cfg->value->data,item,item?item->valid:0);
			if(!item || item->valid)
			{
				//wtk_debug("[%.*s]=[%.*s]\n",cfg->tok->pos,cfg->tok->data,cfg->value->pos,cfg->value->data);
				wtk_cfg_queue_add_string(cfg->cur->cfg,cfg->tok->data,cfg->tok->pos,cfg->value->data,cfg->value->pos);
			}
			cfg->state=CF_EXPR_START;
		}
	}else if(!isspace(c))
	{
		wtk_debug("expect \";\"\n");
		ret=-1;
	}
	return ret;
}

void wtk_cfg_file_set_state(wtk_cfg_file_t *cfg,wtk_cfg_file_state_t state)
{
	cfg->state=state;
	cfg->quoted=0;
	cfg->escaped=0;
	cfg->sub_state=-1;
}

int wtk_cfg_file_feed_array_tok_end(wtk_cfg_file_t *cfg,char c)
{
	int ret=0;

	//wtk_debug("[%c]\n",c);
	if(c==',')
	{
		wtk_cfg_file_set_state(cfg,CFG_ARRAY_START);
	}else if (c==']')
	{
		wtk_cfg_file_set_state(cfg,CF_EXPR_START);
	}else if(!isspace(c))
	{
		wtk_debug("expect array tok like \",\" or \"]\",buf found[%c]\n",c);
		ret=-1;
	}
	return ret;
}

int wtk_cfg_file_feed_array_tok_start(wtk_cfg_file_t *cfg,char c)
{
	wtk_string_t *s;
	int ret=0;

	//wtk_debug("c=[%c]\n",c);
	if(cfg->escaped)
	{
		wtk_strbuf_push_c(cfg->value,c);
		cfg->escaped=0;
		return 0;
	}
	if(cfg->quoted)
	{
		if(c==cfg->quoted_char)
		{
			s=wtk_heap_dup_string(cfg->heap,cfg->value->data,cfg->value->pos+1);
			--s->len;
			s->data[s->len]=0;
			((wtk_string_t**)wtk_array_push(cfg->array))[0]=s;
			//wtk_debug("[%.*s]\n",s->len,s->data);
			//cfg->state=CFG_ARRAY_TOK_END;
			wtk_cfg_file_set_state(cfg,CFG_ARRAY_TOK_END);
		}else
		{
			if(c=='\\')
			{
				cfg->escaped=1;
			}else
			{
				wtk_strbuf_push_c(cfg->value,c);
			}
		}
	}else
	{
		if(isspace(c)||c==','||c==']')
		{
			if(cfg->value->pos>0)
			{
				s=wtk_heap_dup_string(cfg->heap,cfg->value->data,cfg->value->pos+1);
				--s->len;
				s->data[s->len]=0;
				((wtk_string_t**)wtk_array_push(cfg->array))[0]=s;
			}
			//cfg->state=CFG_ARRAY_TOK_END;
			wtk_cfg_file_set_state(cfg,CFG_ARRAY_TOK_END);
			if(!isspace(c))
			{
				ret=wtk_cfg_file_feed_array_tok_end(cfg,c);
			}
		}else if(c=='$')
		{
			cfg->var_cache_state=CFG_ARRAY_TOK_START;
			//cfg->state=CF_VAR_START;
			wtk_cfg_file_set_state(cfg,CF_VAR_START);
		}else
		{
			if(cfg->value->pos==0 && (c=='\'' || c=='"'))
			{
				cfg->quoted=1;
				cfg->quoted_char=c;
			}else
			{
				wtk_strbuf_push_c(cfg->value,c);
			}
		}
	}
	return ret;
}

int wtk_cfg_file_feed_array_start(wtk_cfg_file_t *cfg,char c)
{
	int ret;

	if(!isspace(c))
	{
		//cfg->state=CFG_ARRAY_TOK_START;
		wtk_strbuf_reset(cfg->value);
		//cfg->quoted=0;
		wtk_cfg_file_set_state(cfg,CFG_ARRAY_TOK_START);
		ret=wtk_cfg_file_feed_array_tok_start(cfg,c);
	}else
	{
		ret=0;
	}
	return ret;
}

int wtk_cfg_file_feed_expr_value_start(wtk_cfg_file_t *cfg,char c)
{
	wtk_heap_t *h=cfg->heap;
	wtk_local_cfg_t *lc;
	int ret=0;

	if(c=='{')
	{
		wtk_cfg_item_t* item;

		item=wtk_cfg_queue_find(cfg->cur->cfg,cfg->tok->data,cfg->tok->pos);
		if(item && item->type==WTK_CFG_LC)
		{
			lc=item->value.cfg;
		}else
		{
			lc=wtk_local_cfg_new_h(h);
			wtk_cfg_queue_add_lc(cfg->cur->cfg,cfg->tok->data,cfg->tok->pos,lc);
			wtk_queue_push(&(cfg->cfg_queue),&(lc->q_n));
			lc->parent=cfg->cur;
		}
		cfg->cur=lc;
		cfg->state=CF_EXPR_START;
		++cfg->scope;
		ret=wtk_cfg_file_feed_expr_start(cfg,c);
	}else if(c=='[')
	{
		cfg->state=CFG_ARRAY_START;
		cfg->array=wtk_array_new_h(cfg->heap,5,sizeof(wtk_string_t*));
		wtk_cfg_queue_add_array(cfg->cur->cfg,cfg->tok->data,cfg->tok->pos,cfg->array);
	}else if(is_char(c)||c=='$'||c=='"')
	{
		cfg->state=CF_EXPR_VALUE_TOK_START;
		wtk_strbuf_reset(cfg->value);
		cfg->quoted=c=='"';
		if(cfg->quoted)
		{
			cfg->quoted_char=c;
		}else
		{
			ret=wtk_cfg_file_feed_expr_value_tok_start(cfg,c);
		}
	}else if(!isspace(c))
	{
		wtk_debug("expect expr value start %c.\n",c);
		ret=-1;
	}
	return ret;
}

int wtk_cfg_file_feed_var_tok_start(wtk_cfg_file_t *cfg,char c)
{
	int ret=0;
	wtk_string_t *n;

	if(is_char(c))
	{
		wtk_strbuf_push_c(cfg->var,c);
	}else if(c=='}')
	{
		n=wtk_local_cfg_find_string(cfg->cur,cfg->var->data,cfg->var->pos);
		if(n)
		{
			wtk_strbuf_push(cfg->value,n->data,n->len);
			cfg->state=cfg->var_cache_state;
			//cfg->state=CF_EXPR_VALUE_TOK_START;
		}else
		{
			wtk_debug("var %*.*s not found.\n",cfg->var->pos,cfg->var->pos,cfg->var->data);
			ret=-1;
		}
	}else if(!isspace(c))
	{
		wtk_debug("expect expr tok start.\n");
		ret=-1;
	}
	return ret;
}

int wtk_cfg_file_feed_var_tok(wtk_cfg_file_t *cfg,char c)
{
	int ret;

	if(!isspace(c))
	{
		cfg->state=CF_VAR_TOK_START;
		wtk_strbuf_reset(cfg->var);
		ret=wtk_cfg_file_feed_var_tok_start(cfg,c);
	}else
	{
		ret=0;
	}
	return ret;
}

int wtk_cfg_file_feed_var_start(wtk_cfg_file_t *cfg,char c)
{
	int ret;

	if(c=='{')
	{
		cfg->state=CF_VAR_TOK;
		ret=0;
	}else
	{
		wtk_debug("expect var { start.\n");
		ret=-1;
	}
	return ret;
}


int wtk_cfg_file_feed_escape_x2(wtk_cfg_file_t *cfg,char c)
{
	int ret;
	int v;

	v=wtk_char_to_hex(c);
	if(v==-1)
	{
		ret=-1;
	}else
	{
		cfg->escape_char=(cfg->escape_char<<4)+v;
		wtk_strbuf_push_c(cfg->value,cfg->escape_char);
		cfg->state=CF_EXPR_VALUE_TOK_START;
		ret=0;
	}
	return ret;
}

int wtk_cfg_file_feed_escape_x1(wtk_cfg_file_t *cfg,char c)
{
	int ret;
	int v;

	v=wtk_char_to_hex(c);
	if(v==-1)
	{
		ret=-1;
	}else
	{
		cfg->escape_char=v;
		cfg->state=CFG_ESCAPE_X2;
		ret=0;
	}
	return ret;
}

int wtk_cfg_file_feed_escape_o2(wtk_cfg_file_t *cfg,char c)
{
	int ret;

	if(c>='0' && c<='7')
	{
		cfg->escape_char=(cfg->escape_char<<3)+c-'0';
		wtk_strbuf_push_c(cfg->value,cfg->escape_char);
		cfg->state=CF_EXPR_VALUE_TOK_START;
		ret=0;
	}else
	{
		ret=-1;
	}
	return ret;
}

int wtk_cfg_file_feed_escape_o1(wtk_cfg_file_t *cfg,char c)
{
	int ret;

	if(c>='0' && c<='7')
	{
		cfg->escape_char=(cfg->escape_char<<3)+c-'0';
		cfg->state=CFG_ESCAPE_O2;
		ret=0;
	}else
	{
		ret=-1;
	}
	return ret;
}

int wtk_cfg_file_feed_escape_start(wtk_cfg_file_t *cfg,char c)
{
	if(c=='x'||c=='X')
	{
		cfg->escape_char=0;
		cfg->state=CFG_ESCAPE_X1;
	}else if(c>='0' && c<='7')
	{
		cfg->escape_char=c-'0';
		cfg->state=CFG_ESCAPE_O1;
	}else
	{
		switch(c)
		{
		case 't':
			wtk_strbuf_push_c(cfg->value,'\t');
			break;
		case 'n':
			wtk_strbuf_push_c(cfg->value,'\n');
			break;
		case 'r':
			wtk_strbuf_push_c(cfg->value,'\r');
			break;
		case '\'':
			wtk_strbuf_push_c(cfg->value,'\'');
			break;
		case '\"':
			wtk_strbuf_push_c(cfg->value,'\"');
			break;
		case '\\':
			wtk_strbuf_push_c(cfg->value,'\\');
			break;
		default:
			wtk_strbuf_push_c(cfg->value,c);
			break;
		}
		cfg->state=CF_EXPR_VALUE_TOK_START;
	}
	return 0;
}

int wtk_cfg_file_feed_expr_value_tok_start(wtk_cfg_file_t *cfg,char c)
{
	int ret=0;

	if(c=='\\')
	{
		cfg->state=CFG_ESCAPE_START;
		return 0;
	}
	if(cfg->quoted)
	{
		if(c==cfg->quoted_char)
		{
			cfg->quoted=0;
			cfg->state=CF_EXPR_VALUE_TOK_END;
		}else
		{
			wtk_strbuf_push_c(cfg->value,c);
		}
		return 0;
	}
	if(is_value(c))
	{
		wtk_strbuf_push_c(cfg->value,c);
	}else if(c==';')
	{
		cfg->state=CF_EXPR_VALUE_TOK_END;
		ret=wtk_cfg_file_feed_expr_value_tok_end(cfg,c);
	}else if(c=='$')
	{
		cfg->var_cache_state=CF_EXPR_VALUE_TOK_START;
		cfg->state=CF_VAR_START;
	}else
	{
		wtk_debug("expect var value %c end.\n",c);
		ret=-1;
	}
	return ret;
}



void wtk_cfg_file_push_condition(wtk_cfg_file_t *cfg)
{
	wtk_heap_t *heap=cfg->heap;
	wtk_cfg_if_item_t *item;

	item=(wtk_cfg_if_item_t*)wtk_heap_malloc(heap,sizeof(wtk_cfg_if_item_t));
	item->_or=0;
	item->valid=1;
	wtk_queue_init(&(item->cond));
	wtk_queue_push(&(cfg->if_q),&(item->q_n));
}

void wtk_cfg_file_pop_condition(wtk_cfg_file_t *cfg)
{
	wtk_queue_pop(&(cfg->if_q));
}

wtk_cfg_if_item_t* wtk_cfg_file_get_last_if_item(wtk_cfg_file_t *cfg)
{
	if(cfg->if_q.length>0)
	{
		return (wtk_cfg_if_item_t*)data_offset2(cfg->if_q.push,wtk_cfg_if_item_t,q_n);
	}else
	{
		return NULL;
	}
}

wtk_cfg_if_cond_t* wtk_cfg_file_get_last_if_cond(wtk_cfg_file_t *cfg)
{
	wtk_cfg_if_item_t *item;

	item=wtk_cfg_file_get_last_if_item(cfg);
	return (wtk_cfg_if_cond_t*)data_offset2(item->cond.push,wtk_cfg_if_cond_t,q_n);
}

wtk_cfg_if_cond_t* wtk_cfg_file_pop_item_cond(wtk_cfg_file_t *cfg)
{
	wtk_heap_t *heap=cfg->heap;
	wtk_cfg_if_cond_t *cond;
	wtk_cfg_if_item_t *item;

	item=data_offset2(cfg->if_q.push,wtk_cfg_if_item_t,q_n);
	cond=(wtk_cfg_if_cond_t*)wtk_heap_malloc(heap,sizeof(wtk_cfg_if_cond_t));
	cond->k=NULL;
	cond->v=NULL;
	wtk_queue_push(&(item->cond),&(cond->q_n));
	return cond;
}

#include "wtk/core/wtk_str_parser.h"


wtk_string_t* wtk_cfg_file_get_var(wtk_cfg_file_t *cfg,char *k,int k_len)
{
	wtk_cfg_item_t *item;

	item=wtk_cfg_queue_find(cfg->cur->cfg,k,k_len);
	if(item && item->type==WTK_CFG_STRING)
	{
		return item->value.str;
	}else
	{
		return NULL;
	}
}

char* wtk_cfg_file_str_expand(wtk_cfg_file_t *cfg,char *s,int len)
{
	wtk_string_parser2_t *p;

	p=wtk_string_parser2_new();
	wtk_string_parser2_set(p,cfg,(wtk_string_parser2_get_var_f)wtk_cfg_file_get_var);
	wtk_string_parser2_process(p,s,len);
	//wtk_debug("[%.*s]\n",p->buf->pos,p->buf->data);
	//exit(0);
	if(p->buf->pos>0)
	{
		s=wtk_data_to_str(p->buf->data,p->buf->pos);
	}else
	{
		s=NULL;
	}
	wtk_string_parser2_delete(p);
	return s;
}

void wtk_cfg_file_update_last_if(wtk_cfg_file_t *cfg)
{
	wtk_cfg_if_item_t *item;
	wtk_cfg_if_cond_t *cond;
	wtk_queue_node_t *qn;
	wtk_cfg_item_t *ci;
	char *s;
	int ret;

	item=wtk_cfg_file_get_last_if_item(cfg);
	if(item->_or)
	{
		item->valid=0;
		for(qn=item->cond.pop;qn;qn=qn->next)
		{
			cond=data_offset2(qn,wtk_cfg_if_cond_t,q_n);
			//wtk_debug("[%.*s]\n",cond->k->len,cond->k->data);
			ci=wtk_local_cfg_find(cfg->cur,cond->k->data,cond->k->len);
			//wtk_debug("ci=%p\n",ci);
			if(ci)
			{
				if(!cond->v)
				{
					item->valid=1;
					goto end;
				}else if(ci->type==WTK_CFG_STRING && wtk_string_cmp(cond->v,ci->value.str->data,ci->value.str->len)==0)
				{
					item->valid=1;
					goto end;
				}
			}else if(cond->k->data[0]=='#')
			{
				s=wtk_cfg_file_str_expand(cfg,cond->k->data+1,cond->k->len-1);
				if(s)
				{
					ret=wtk_file_exist(s);
					//wtk_debug("[%s]=%d\n",s,ret);
					wtk_free(s);
				}else
				{
					ret=-1;
				}
				if(ret==0)
				{
					item->valid=1;
					goto end;
				}
				//exit(0);
			}
		}
	}else
	{
		item->valid=1;
		for(qn=item->cond.pop;qn;qn=qn->next)
		{
			cond=data_offset2(qn,wtk_cfg_if_cond_t,q_n);
			//wtk_debug("[%.*s]\n",cond->k->len,cond->k->data);
			ci=wtk_local_cfg_find(cfg->cur,cond->k->data,cond->k->len);
			//wtk_debug("ci=%p\n",ci);
			if(!ci)
			{
				//wtk_debug("[%.*s]\n",cond->k->len,cond->k->data);
				if(cond->k->data[0]=='#')
				{
					s=wtk_cfg_file_str_expand(cfg,cond->k->data+1,cond->k->len-1);
					if(s)
					{
						ret=wtk_file_exist(s);
						//wtk_debug("[%s]=%d\n",s,ret);
						wtk_free(s);
					}else
					{
						ret=-1;
					}
					//wtk_debug("[%.*s]=%d\n",cond->k->len,cond->k->data,ret);
				}else
				{
					ret=-1;
				}
				if(ret!=0)
				{
					item->valid=0;
					goto end;
				}
			}else if(cond->v)
			{
				if(ci->type!=WTK_CFG_STRING)
				{
					item->valid=0;
					goto end;
				}else if(wtk_string_cmp(cond->v,ci->value.str->data,ci->value.str->len)!=0)
				{
					item->valid=0;
					goto end;
				}
			}
		}
	}
end:
	return;
}

int wtk_cfg_file_feed_if(wtk_cfg_file_t* cfg,char c)
{
enum
{
	WTK_CFG_IF_INIT=0,
	WTK_CFG_IF_KEY,
	WTK_CFG_IF_VALUE,
};
	wtk_strbuf_t *buf=cfg->tok;
	wtk_cfg_if_item_t *item;
	wtk_cfg_if_cond_t *cond;
	wtk_heap_t *heap=cfg->heap;

	if(cfg->sub_state==-1)
	{
		cfg->sub_state=WTK_CFG_IF_INIT;
	}
	switch(cfg->sub_state)
	{
	case WTK_CFG_IF_INIT:
		if(!isspace(c))
		{
			wtk_strbuf_reset(buf);
			wtk_strbuf_push_c(buf,c);
			cfg->sub_state=WTK_CFG_IF_KEY;
		}
		break;
	case WTK_CFG_IF_KEY:
		if(c=='=')
		{
			cond=wtk_cfg_file_pop_item_cond(cfg);
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			cond->k=wtk_heap_dup_string(heap,buf->data,buf->pos);
			cfg->sub_state=WTK_CFG_IF_VALUE;
			wtk_strbuf_reset(buf);
		}else if(isspace(c))
		{
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			if(wtk_str_equal_s(buf->data,buf->pos,"or"))
			{
				item=wtk_cfg_file_get_last_if_item(cfg);
				item->_or=1;
			}else if(wtk_str_equal_s(buf->data,buf->pos,"and"))
			{
				item=wtk_cfg_file_get_last_if_item(cfg);
				item->_or=0;
			}else if(wtk_str_equal_s(buf->data,buf->pos,"then"))
			{
				wtk_cfg_file_update_last_if(cfg);
				wtk_cfg_file_set_state(cfg,CF_EXPR_START);
			}else
			{
				cond=wtk_cfg_file_pop_item_cond(cfg);
				//wtk_debug("[%.*s]\n",buf->pos,buf->data);
				cond->k=wtk_heap_dup_string(heap,buf->data,buf->pos);
			}
			wtk_strbuf_reset(buf);
			cfg->sub_state=WTK_CFG_IF_INIT;
		}else
		{
			wtk_strbuf_push_c(buf,c);
		}
		break;
	case WTK_CFG_IF_VALUE:
		if(isspace(c))
		{
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			if(buf->data[0]=='"' && buf->data[buf->pos-1]=='\"')
			{
				wtk_strbuf_pop(buf,NULL,1);
				buf->pos-=1;
			}
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			cond=wtk_cfg_file_get_last_if_cond(cfg);
			cond->v=wtk_heap_dup_string(heap,buf->data,buf->pos);
			cfg->sub_state=WTK_CFG_IF_INIT;
			wtk_strbuf_reset(buf);
		}else
		{
			wtk_strbuf_push_c(buf,c);
		}
		break;
	}
	return 0;
}


int wtk_cfg_file_feed_expr_tok_start(wtk_cfg_file_t* cfg,char c)
{
	wtk_cfg_if_item_t *item;
	int ret=0;

	if(cfg->quoted)
	{
		if(c!=cfg->quoted_char)
		{
			wtk_strbuf_push_c(cfg->tok,c);
		}else
		{
			cfg->quoted=0;
			cfg->state=CF_EXPR_TOK_WAIT_EQ;
		}
	}else
	{
		if(is_char(c))
		{
			wtk_strbuf_push_c(cfg->tok,c);
		}else if(c=='=')
		{
			cfg->state=CF_EXPR_VALUE_START;
			//ret=wtk_cfg_file_feed_expr_value_start(cfg,c);
		}else if(!isspace(c))
		{
			ret=-1;
		}else if(cfg->tok->pos>0)
		{
			if(wtk_str_equal_s(cfg->tok->data,cfg->tok->pos,"if"))
			{
				//wtk_debug("[%.*s]\n",cfg->tok->pos,cfg->tok->data);
				wtk_cfg_file_push_condition(cfg);
				wtk_strbuf_reset(cfg->tok);
				//cfg->state=CFG_EXPR_IF;
				wtk_cfg_file_set_state(cfg,CFG_EXPR_IF);
			}else if(wtk_str_equal_s(cfg->tok->data,cfg->tok->pos,"else"))
			{
				//wtk_debug("[%.*s]\n",cfg->tok->pos,cfg->tok->data);
				item=wtk_cfg_file_get_last_if_item(cfg);
				item->valid=!item->valid;
				wtk_strbuf_reset(cfg->tok);
				wtk_cfg_file_set_state(cfg,CF_EXPR_START);
			}else if(wtk_str_equal_s(cfg->tok->data,cfg->tok->pos,"end"))
			{
				wtk_cfg_file_pop_condition(cfg);
				wtk_strbuf_reset(cfg->tok);
				wtk_cfg_file_set_state(cfg,CF_EXPR_START);
			}
		}
	}
	return ret;
}

int wtk_cfg_file_feed_expr_tok_wait_eq(wtk_cfg_file_t* cfg,char c)
{
	if(c=='=')
	{
		cfg->state=CF_EXPR_VALUE_START;
	}
	return 0;
}


int wtk_cfg_file_feed_comment(wtk_cfg_file_t *cfg,char c)
{
	int len=sizeof("include")-1;
	wtk_strbuf_t *buf;

	if(c=='\n')
	{
		//wtk_strbuf_reset(cfg->tok);
		cfg->state=CF_EXPR_START;
	}else
	{
		buf=cfg->tok;
		if(buf->pos<len)
		{
			wtk_strbuf_push_c(buf,c);
			if(buf->pos==len)
			{
				if(strncmp(buf->data,"include",buf->pos)==0)
				{
					cfg->state=CF_EXPR_VALUE_START;
					cfg->included=1;
				}
			}
		}
	}
	return 0;
}

int wtk_cfg_file_feed_expr_start(wtk_cfg_file_t* cfg,char c)
{
	int ret=0;

#ifdef WIN32
	if(is_char((unsigned char)c) ||c=='"' ||c=='\'')
#else
	if(is_char(c) ||c=='"' ||c=='\'')
#endif
	{
		cfg->state=CF_EXPR_TOK_START;
		wtk_strbuf_reset(cfg->tok);
		if(c=='"' ||c=='\'')
		{
			cfg->quoted=1;
			cfg->quoted_char=c;
		}else
		{
			ret=wtk_cfg_file_feed_expr_tok_start(cfg,c);
		}
	}else if(c=='}')
	{
		if(cfg->scope<=0)
		{
			ret=-1;
		}else
		{
			--cfg->scope;
			cfg->cur=cfg->cur->parent;
		}
	}else if(c=='#')
	{
		cfg->state=CFG_COMMENT;
		wtk_strbuf_reset(cfg->tok);
	}else
	{
		//wtk_debug("c=%c\n",c);
		ret=0;
	}
	return ret;
}

int wtk_cfg_file_feed_fn(wtk_cfg_file_t* cfg,char *fn)
{
	char *data=0;
	int len,ret=-1;

	data=file_read_buf(fn,&len);
	if(!data){goto end;}
	//print_data(data,len);
	ret=wtk_cfg_file_feed(cfg,data,len);
end:
	if(data){free(data);}
	return ret;
}

int wtk_cfg_file_feed_string(wtk_cfg_file_t *c,char *d,int bytes)
{
	char *s=d,*e=d+bytes;
	int ret=-1;

	while(s<e)
	{
		//printf("%c",*s);
		//wtk_debug("[%c]:%d\n",*s,c->state);
		switch(c->state)
		{
		case CF_EXPR_START:
			ret=wtk_cfg_file_feed_expr_start(c,*s);
			break;
		case CF_EXPR_TOK_START:
			ret=wtk_cfg_file_feed_expr_tok_start(c,*s);
			break;
		case CF_EXPR_TOK_WAIT_EQ:
			ret=wtk_cfg_file_feed_expr_tok_wait_eq(c,*s);
			break;
		case CF_EXPR_VALUE_START:
			ret=wtk_cfg_file_feed_expr_value_start(c,*s);
			break;
		case CFG_ESCAPE_START:
			ret=wtk_cfg_file_feed_escape_start(c,*s);
			break;
		case CFG_ESCAPE_X1:
			ret=wtk_cfg_file_feed_escape_x1(c,*s);
			break;
		case CFG_ESCAPE_X2:
			ret=wtk_cfg_file_feed_escape_x2(c,*s);
			break;
		case CFG_ESCAPE_O1:
			ret=wtk_cfg_file_feed_escape_o1(c,*s);
			break;
		case CFG_ESCAPE_O2:
			ret=wtk_cfg_file_feed_escape_o2(c,*s);
			break;
		case CF_EXPR_VALUE_TOK_START:
			ret=wtk_cfg_file_feed_expr_value_tok_start(c,*s);
			break;
		case CF_EXPR_VALUE_TOK_END:
			ret=wtk_cfg_file_feed_expr_value_tok_end(c,*s);
			break;
		case CF_VAR_START:
			ret=wtk_cfg_file_feed_var_start(c,*s);
			break;
		case CF_VAR_TOK:
			ret=wtk_cfg_file_feed_var_tok(c,*s);
			break;
		case CF_VAR_TOK_START:
			ret=wtk_cfg_file_feed_var_tok_start(c,*s);
			break;
		case CFG_ARRAY_START:
			ret=wtk_cfg_file_feed_array_start(c,*s);
			break;
		case CFG_ARRAY_TOK_START:
			ret=wtk_cfg_file_feed_array_tok_start(c,*s);
			break;
		case CFG_ARRAY_TOK_END:
			ret=wtk_cfg_file_feed_array_tok_end(c,*s);
			break;
		case CFG_COMMENT:
			ret=wtk_cfg_file_feed_comment(c,*s);
			break;
		case CFG_EXPR_IF:
			ret=wtk_cfg_file_feed_if(c,*s);
			break;
		default:
			ret=-1;
			break;
		}
		if(ret!=0)
		{
			print_data(d,s-d);
			break;
		}
		++s;
	}
	//wtk_cfg_file_print(c);
	return ret;
}

int wtk_cfg_file_feed(wtk_cfg_file_t *c,char *d,int bytes)
{
	int ret;

	//wtk_debug("%*.*s\n",bytes,bytes,d);
	c->state=CF_EXPR_START;
	c->cur=c->main;
	ret=wtk_cfg_file_feed_string(c,d,bytes);
	return ret;
}


void wtk_cfg_file_print(wtk_cfg_file_t *c)
{
	/*
	wtk_queue_node_t *n;
	wtk_local_cfg_t *lc;

	for(n=c->cfg_queue.pop;n;n=n->next)
	{
		lc=data_offset(n,wtk_local_cfg_t,q_n);
		wtk_local_cfg_print(lc);
	}
	*/
	wtk_local_cfg_print(c->main);

}
