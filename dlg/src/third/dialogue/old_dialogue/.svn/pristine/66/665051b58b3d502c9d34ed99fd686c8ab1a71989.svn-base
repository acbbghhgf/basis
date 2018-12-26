#include <ctype.h>
#include "wtk_str_parser.h" 

void wtk_string_parser_init(wtk_string_parser_t *p,wtk_strbuf_t *buf)
{
	wtk_strbuf_reset(buf);
	p->buf=buf;
	p->sub_state=0;
	p->quoted=0;
}

/**
 * return 1 for end,0 for continue;
 */
int wtk_string_parse(wtk_string_parser_t *p,char *data,int bytes)
{
enum
{
	WTK_STRING_INIT=0,
	WTK_STRING_WAIT_END,
	WTK_STRING_WAIT_QUOTE_END,
	WTK_STRING_ESC,
	WTK_STRING_ESC_X1,
	WTK_STRING_ESC_X2,
};
	wtk_strbuf_t *buf=p->buf;
	wtk_string_t v;
	char c;
	int ret;

	wtk_string_set(&(v),data,bytes);
	switch(p->sub_state)
	{
	case WTK_STRING_INIT:
		if(v.len>1 || !isspace(v.data[0]))
		{
			wtk_strbuf_reset(buf);
			if(v.len==1 &&(v.data[0]=='\'' || v.data[0]=='"'))
			{
				p->quoted=1;
				p->quoted_char=v.data[0];
				p->sub_state=WTK_STRING_WAIT_QUOTE_END;
			}else
			{
				p->quoted=0;
				//wtk_strbuf_push(buf,v->data,v->len);
				p->sub_state=WTK_STRING_WAIT_END;
				wtk_string_parse(p,data,bytes);
			}
		}
		break;
	case WTK_STRING_WAIT_END:
		if(v.len==1)
		{
			c=v.data[0];
			//wtk_debug("%c\n",c);
			if(c=='\\')
			{
				p->sub_state=WTK_STRING_ESC;
			}else if(isspace(c) || c=='=' || c==')' || c=='('||c=='['||
					c==']' || c=='{'|| c=='}' || c=='<'|| c=='>' || c==';' || c==',' ||c=='/')
			{
				//wtk_debug("[%.*s]\n",buf->pos,buf->data);
				//wtk_lexc_restore_string_state(l);
				//wtk_lexc_feed(l,v);
				return 2;
			}else
			{
				wtk_strbuf_push(buf,v.data,v.len);
			}
		}else
		{
			wtk_strbuf_push(buf,v.data,v.len);
		}
		break;
	case WTK_STRING_WAIT_QUOTE_END:
		if(v.len==1)
		{
			if(v.data[0]=='\\')
			{
				p->sub_state=WTK_STRING_ESC;
			}else if(v.data[0]==p->quoted_char)
			{
				//wtk_debug("[%.*s]\n",buf->pos,buf->data);
				//exit(0);
				//wtk_lexc_restore_string_state(l);
				return 1;
			}else
			{
				wtk_strbuf_push(buf,v.data,v.len);
			}
		}else
		{
			wtk_strbuf_push(buf,v.data,v.len);
		}
		break;
	case WTK_STRING_ESC:
		if(v.len==1)
		{
			c=v.data[0];
			switch(c)
			{
			case 'x':
			case 'X':
				p->sub_state=WTK_STRING_ESC_X1;
				break;
			case 'n':
				wtk_strbuf_push_c(buf,'\n');
				p->sub_state=p->quoted?WTK_STRING_WAIT_QUOTE_END:WTK_STRING_WAIT_END;
				break;
			case 'r':
				wtk_strbuf_push_c(buf,'\r');
				p->sub_state=p->quoted?WTK_STRING_WAIT_QUOTE_END:WTK_STRING_WAIT_END;
				break;
			case 't':
				wtk_strbuf_push_c(buf,'\t');
				p->sub_state=p->quoted?WTK_STRING_WAIT_QUOTE_END:WTK_STRING_WAIT_END;
				break;
			case 'b':
				wtk_strbuf_push_c(buf,'\b');
				p->sub_state=p->quoted?WTK_STRING_WAIT_QUOTE_END:WTK_STRING_WAIT_END;
				break;
			case 'f':
				wtk_strbuf_push_c(buf,'\f');
				p->sub_state=p->quoted?WTK_STRING_WAIT_QUOTE_END:WTK_STRING_WAIT_END;
				break;
			case '\\':
				wtk_strbuf_push_c(buf,'\\');
				p->sub_state=p->quoted?WTK_STRING_WAIT_QUOTE_END:WTK_STRING_WAIT_END;
				break;
			default:
				wtk_strbuf_push_c(buf,c);
				p->sub_state=p->quoted?WTK_STRING_WAIT_QUOTE_END:WTK_STRING_WAIT_END;
				break;
			}
		}else
		{
			wtk_strbuf_push(buf,v.data,v.len);
			p->sub_state=p->quoted?WTK_STRING_WAIT_QUOTE_END:WTK_STRING_WAIT_END;
		}
		break;
	case WTK_STRING_ESC_X1:
		if(v.len>1)
		{
			return -1;
		}
		c=v.data[0];
		ret=wtk_char_to_hex(c);
		if(ret==-1){return -1;}
		p->hex1=ret;
		p->sub_state=WTK_STRING_ESC_X2;
		break;
	case WTK_STRING_ESC_X2:
		if(v.len>1)
		{
			return -1;
		}
		c=v.data[0];
		ret=wtk_char_to_hex(c);
		if(ret==-1){return -1;}
		c=(p->hex1<<4)+ret;
		wtk_strbuf_push_c(buf,c);
		p->sub_state=p->quoted?WTK_STRING_WAIT_QUOTE_END:WTK_STRING_WAIT_END;
		break;
	}
	return 0;
}

void wtk_var_parse_init(wtk_var_parse_t *p,wtk_strbuf_t *buf)
{
	p->buf=buf;
	p->sub_state=0;
	wtk_strbuf_reset(buf);
}

wtk_var_parse_t* wtk_var_parse_new()
{
	wtk_var_parse_t *p;

	p=(wtk_var_parse_t*)wtk_malloc(sizeof(wtk_var_parse_t));
	p->buf=wtk_strbuf_new(256,1);
	p->sub_state=0;
	return p;
}

void wtk_var_parse_delete(wtk_var_parse_t *p)
{
	wtk_strbuf_delete(p->buf);
	wtk_free(p);
}

void wtk_var_parse_reset(wtk_var_parse_t *p)
{
	p->sub_state=0;
	wtk_strbuf_reset(p->buf);
}

int wtk_var_parse2(wtk_var_parse_t *p,char *data,int bytes)
{
enum
{
	WTK_STRING_VAR_INIT=0,
	WTK_STRING_VAR_DOLLAR,
	WTK_STRING_VAR_WAIT_VAR,
	WTK_STRING_VAR_VAR,
	WTK_STRING_VAR_VAR_WAIT_END,
};
	wtk_strbuf_t *buf=p->buf;
	char c;
	wtk_string_t v;

	wtk_string_set(&(v),data,bytes);
	switch(p->sub_state)
	{
	case WTK_STRING_VAR_INIT:
		if(v.len==1 && v.data[0]=='$')
		{
			p->sub_state=WTK_STRING_VAR_DOLLAR;
			return 0;
		}
		break;
	case WTK_STRING_VAR_DOLLAR:
		if(v.len==1 && v.data[0]=='{')
		{
			p->sub_state=WTK_STRING_VAR_WAIT_VAR;
			return 0;
		}
		break;
	case WTK_STRING_VAR_WAIT_VAR:
		if(v.len>1 || !isspace(v.data[0]))
		{
			wtk_strbuf_reset(buf);
			wtk_strbuf_push(buf,v.data,v.len);
			p->sub_state=WTK_STRING_VAR_VAR;
		}
		break;
	case WTK_STRING_VAR_VAR:
		c=v.data[0];
		if(v.len==1 && (c=='}' || isspace(c)))
		{
			if(c=='}')
			{
				return 1;
			}else
			{
				p->sub_state=WTK_STRING_VAR_VAR_WAIT_END;
			}
		}else
		{
			wtk_strbuf_push(buf,v.data,v.len);
		}
		break;
	case WTK_STRING_VAR_VAR_WAIT_END:
		if(v.len==1 && v.data[0]=='}')
		{
			return 1;
		}
		break;
	}
	return 0;
}


int wtk_var_parse(wtk_var_parse_t *p,char *data,int bytes)
{
enum
{
	WTK_STRING_DOLLAR=0,
	WTK_STRING_WAIT_VAR,
	WTK_STRING_VAR,
	WTK_STRING_VAR_WAIT_END,
};
	wtk_strbuf_t *buf=p->buf;
	char c;
	wtk_string_t v;

	wtk_string_set(&(v),data,bytes);
	switch(p->sub_state)
	{
	case WTK_STRING_DOLLAR:
		if(v.len==1 && v.data[0]=='{')
		{
			p->sub_state=WTK_STRING_WAIT_VAR;
			return 0;
		}
		break;
	case WTK_STRING_WAIT_VAR:
		if(v.len>1 || !isspace(v.data[0]))
		{
			wtk_strbuf_reset(buf);
			wtk_strbuf_push(buf,v.data,v.len);
			p->sub_state=WTK_STRING_VAR;
		}
		break;
	case WTK_STRING_VAR:
		c=v.data[0];
		if(v.len==1 && (c=='}' || isspace(c)))
		{
			if(c=='}')
			{
				return 1;
			}else
			{
				p->sub_state=WTK_STRING_VAR_WAIT_END;
			}
		}else
		{
			wtk_strbuf_push(buf,v.data,v.len);
		}
		break;
	case WTK_STRING_VAR_WAIT_END:
		if(v.len==1 && v.data[0]=='}')
		{
			return 1;
		}
		break;
	}
	return 0;
}

void wtk_strkv_parser_init(wtk_strkv_parser_t *p,char *data,int len)
{
	p->state=WTK_STRKV_PARSER_STATE_INIT;
	p->s=data;
	p->e=data+len;
	wtk_string_set(&(p->k),0,0);
	wtk_string_set(&(p->v),0,0);
	//wtk_debug("[%.*s]\n",len,data);
}

int wtk_strkv_parser_is_end(wtk_strkv_parser_t *p)
{
	return p->s>=p->e?1:0;
	//return p->state==WTK_STRKV_PARSER_STATE_INIT?0:1;
}

int wtk_strkv_parser_next(wtk_strkv_parser_t *p)
{
	char c;
	int n;
	int ret;

	wtk_string_set(&(p->k),0,0);
	wtk_string_set(&(p->v),0,0);
	while(p->s<p->e)
	{
		c=*(p->s);
		n=wtk_utf8_bytes(c);
		//wtk_debug("state=%d %.*s\n",p->state,n,p->s);
		switch(p->state)
		{
		case WTK_STRKV_PARSER_STATE_INIT:
			if(n>1 || !isspace(c))
			{
				p->k.data=p->s;
				p->state=WTK_STRKV_PARSER_STATE_KEY;
			}
			break;
		case WTK_STRKV_PARSER_STATE_KEY:
			if(n==1 && (c=='='||c==',' || isspace(c)))
			{
				p->k.len=p->s-p->k.data;
				//wtk_debug("[%.*s]\n",p->k.len,p->k.data);
				if(c=='=')
				{
					p->state=WTK_STRKV_PARSER_STATE_WAIT_VALUE;
				}else if(c==',')
				{
					p->state=WTK_STRKV_PARSER_STATE_INIT;
					p->s+=n;
					ret=0;
					goto end;
				}else
				{
					p->state=WTK_STRKV_PARSER_STATE_WAIT_EQ;
				}
			}
			break;
		case WTK_STRKV_PARSER_STATE_WAIT_EQ:
			if(n==1)
			{
				if(c==',')
				{
					p->state=WTK_STRKV_PARSER_STATE_INIT;
					p->s+=n;
					ret=0;
					goto end;
				}else if(c=='=')
				{
					p->state=WTK_STRKV_PARSER_STATE_WAIT_VALUE;
				}
			}
			break;
		case WTK_STRKV_PARSER_STATE_WAIT_VALUE:
			if(n>1 || !isspace(c))
			{
				if(n==1 && c=='"')
				{
					p->state=WTK_STRKV_PARSER_STATE_QUOTE_VALUE;
					p->v.data=p->s+1;
				}else
				{
					p->state=WTK_STRKV_PARSER_STATE_VALUE;
					p->v.data=p->s;
				}
			}
			break;
		case WTK_STRKV_PARSER_STATE_VALUE:
			if(n==1 && (c==',' || isspace(c)))
			{
				p->v.len=p->s-p->v.data;
				//wtk_debug("[%.*s]\n",p->v.len,p->v.data);
				if(c==',')
				{
					p->state=WTK_STRKV_PARSER_STATE_INIT;
					p->s+=n;
					ret=0;
					goto end;
				}else
				{
					p->state=WTK_STRKV_PARSER_STATE_WAIT_END;
				}
			}
			break;
		case WTK_STRKV_PARSER_STATE_QUOTE_VALUE:
			if(n==1 && c=='"')
			{
				p->v.len=p->s-p->v.data;
				p->state=WTK_STRKV_PARSER_STATE_WAIT_END;
			}
			break;
		case WTK_STRKV_PARSER_STATE_WAIT_END:
			if(n==1 && c==',')
			{
				p->state=WTK_STRKV_PARSER_STATE_INIT;
				p->s+=n;
				ret=0;
				goto end;
			}else if(p->s+n>=p->e)
			{
				p->state=WTK_STRKV_PARSER_STATE_INIT;
				p->s+=n;
				ret=0;
				goto end;
			}
			break;
		}
		p->s+=n;
	}
	if(p->state==WTK_STRKV_PARSER_STATE_KEY)
	{
		p->k.len=p->s-p->k.data;
		p->state=WTK_STRKV_PARSER_STATE_INIT;
		ret=0;
		goto end;
	}else if(p->state==WTK_STRKV_PARSER_STATE_VALUE)
	{
		if(p->s>=p->e)
		{
			p->v.len=p->s-p->v.data;
			p->state=WTK_STRKV_PARSER_STATE_INIT;
			ret=0;
			goto end;
		}
	}else if(p->state==WTK_STRKV_PARSER_STATE_WAIT_END)
	{
		if(p->s>=p->e)
		{
			p->state=WTK_STRKV_PARSER_STATE_INIT;
			ret=0;
			goto end;
		}
	}
	ret=-1;
end:
	return ret;
}

wtk_string_parser2_t* wtk_string_parser2_new()
{
	wtk_string_parser2_t *p;

	p=(wtk_string_parser2_t*)wtk_malloc(sizeof(wtk_string_parser2_t));
	p->buf=wtk_strbuf_new(256,1);
	p->var=wtk_strbuf_new(64,1);
	p->get_var=NULL;
	p->ths=NULL;
	wtk_string_parser2_reset(p);
	return p;
}

void wtk_string_parser2_delete(wtk_string_parser2_t *p)
{
	wtk_strbuf_delete(p->buf);
	wtk_strbuf_delete(p->var);
	wtk_free(p);
}

void wtk_string_parser2_reset(wtk_string_parser2_t *p)
{
	wtk_strbuf_reset(p->buf);
	wtk_strbuf_reset(p->var);
	p->sub_state=0;
	p->quoted=0;
	p->get_var=NULL;
	p->ths=NULL;
}

void wtk_string_parser2_reset2(wtk_string_parser2_t *p)
{
	wtk_strbuf_reset(p->var);
	p->sub_state=0;
	p->quoted=0;
}

void wtk_string_parser2_set(wtk_string_parser2_t *p,void *ths,wtk_string_parser2_get_var_f get_var)
{
	p->ths=ths;
	p->get_var=get_var;
}


int wtk_string_parse2(wtk_string_parser2_t *p,wtk_string_t *v)
{
enum
{
	WTK_STRING2_INIT=0,
	WTK_STRING2_WAIT_END,
	WTK_STRING2_WAIT_QUOTE_END,
	WTK_STRING2_ESC,
	WTK_STRING2_ESC_X1,
	WTK_STRING2_ESC_X2,
	WTK_STRING2_VAR_S_WAIT,
	WTK_STRING2_VAR_S,
};
	wtk_strbuf_t *buf=p->buf;
	char c;
	int ret;

	//wtk_debug("state=%d [%.*s]\n",p->sub_state,v->len,v->data);
	switch(p->sub_state)
	{
	case WTK_STRING2_INIT:
		if(v->len>1 || !isspace(v->data[0]))
		{
			wtk_strbuf_reset(buf);
			//wtk_debug("char=%c\n",v->data[0]);
			if(v->len==1 &&(v->data[0]=='\'' || v->data[0]=='"'))
			{
				p->quoted=1;
				p->quoted_char=v->data[0];
				p->sub_state=WTK_STRING2_WAIT_QUOTE_END;
			}else if(v->len==1 && v->data[0]=='$')
			{
				p->quoted=0;
				p->sub_state=WTK_STRING2_VAR_S_WAIT;
			}else
			{
				p->quoted=0;
				p->sub_state=WTK_STRING2_WAIT_END;
				return wtk_string_parse2(p,v);
			}
		}
		break;
	case WTK_STRING2_VAR_S_WAIT:
		if(v->len>1 || v->data[0]=='{')
		{
			wtk_strbuf_reset(p->var);
			p->sub_state=WTK_STRING2_VAR_S;
		}else
		{
			wtk_strbuf_push_s(p->var,"$");
			wtk_strbuf_push(p->var,v->data,v->len);
			if(p->quoted)
			{
				p->sub_state=WTK_STRING2_WAIT_QUOTE_END;
			}else
			{
				p->sub_state=WTK_STRING2_WAIT_END;
			}
		}
		break;
	case WTK_STRING2_VAR_S:
		if(v->len==1 && v->data[0]=='}')
		{
			if(p->get_var)
			{
				v=p->get_var(p->ths,p->var->data,p->var->pos);
				if(!v){return -1;}
				wtk_strbuf_push(p->buf,v->data,v->len);
				if(p->quoted)
				{
					p->sub_state=WTK_STRING2_WAIT_QUOTE_END;
				}else
				{
					p->sub_state=WTK_STRING2_WAIT_END;
				}
			}else
			{
				return -1;
			}
			//wtk_debug("[%.*s]\n",p->var->pos,p->var->data);
			//exit(0);
		}else
		{
			wtk_strbuf_push(p->var,v->data,v->len);
		}
		break;
	case WTK_STRING2_WAIT_END:
		if(v->len==1)
		{
			c=v->data[0];
			//wtk_debug("%c\n",c);
			if(c=='\\')
			{
				p->sub_state=WTK_STRING2_ESC;
			}else if(isspace(c) || c=='=' || c==')' || c=='('||c=='['||
					c==']' || c=='{'|| c=='}' || c=='<'|| c=='>' || c==';' || c==',' ||c=='/')
			{
				//wtk_debug("[%.*s]\n",buf->pos,buf->data);
				//wtk_lexc_restore_string_state(l);
				//wtk_lexc_feed(l,v);
				return 2;
			}else
			{
				wtk_strbuf_push(buf,v->data,v->len);
			}
		}else
		{
			wtk_strbuf_push(buf,v->data,v->len);
		}
		break;
	case WTK_STRING2_WAIT_QUOTE_END:
		if(v->len==1)
		{
			if(v->data[0]=='\\')
			{
				p->sub_state=WTK_STRING2_ESC;
			}else if(v->data[0]==p->quoted_char)
			{
				//wtk_debug("[%.*s]\n",buf->pos,buf->data);
				//exit(0);
				//wtk_lexc_restore_string_state(l);
				return 1;
			}else if(v->data[0]=='$')
			{
				p->sub_state=WTK_STRING2_VAR_S_WAIT;
			}
			else
			{
				wtk_strbuf_push(buf,v->data,v->len);
			}
		}else
		{
			wtk_strbuf_push(buf,v->data,v->len);
		}
		break;
	case WTK_STRING2_ESC:
		if(v->len==1)
		{
			c=v->data[0];
			switch(c)
			{
			case 'x':
			case 'X':
				p->sub_state=WTK_STRING2_ESC_X1;
				break;
			case 'n':
				wtk_strbuf_push_c(buf,'\n');
				p->sub_state=p->quoted?WTK_STRING2_WAIT_QUOTE_END:WTK_STRING2_WAIT_END;
				break;
			case 'r':
				wtk_strbuf_push_c(buf,'\r');
				p->sub_state=p->quoted?WTK_STRING2_WAIT_QUOTE_END:WTK_STRING2_WAIT_END;
				break;
			case 't':
				wtk_strbuf_push_c(buf,'\t');
				p->sub_state=p->quoted?WTK_STRING2_WAIT_QUOTE_END:WTK_STRING2_WAIT_END;
				break;
			case 'b':
				wtk_strbuf_push_c(buf,'\b');
				p->sub_state=p->quoted?WTK_STRING2_WAIT_QUOTE_END:WTK_STRING2_WAIT_END;
				break;
			case 'f':
				wtk_strbuf_push_c(buf,'\f');
				p->sub_state=p->quoted?WTK_STRING2_WAIT_QUOTE_END:WTK_STRING2_WAIT_END;
				break;
			case '\\':
				wtk_strbuf_push_c(buf,'\\');
				p->sub_state=p->quoted?WTK_STRING2_WAIT_QUOTE_END:WTK_STRING2_WAIT_END;
				break;
			default:
				wtk_strbuf_push_c(buf,c);
				p->sub_state=p->quoted?WTK_STRING2_WAIT_QUOTE_END:WTK_STRING2_WAIT_END;
				break;
			}
		}else
		{
			wtk_strbuf_push(buf,v->data,v->len);
			p->sub_state=p->quoted?WTK_STRING2_WAIT_QUOTE_END:WTK_STRING2_WAIT_END;
		}
		break;
	case WTK_STRING2_ESC_X1:
		if(v->len>1)
		{
			return -1;
		}
		c=v->data[0];
		ret=wtk_char_to_hex(c);
		if(ret==-1){return -1;}
		p->hex1=ret;
		p->sub_state=WTK_STRING2_ESC_X2;
		break;
	case WTK_STRING2_ESC_X2:
		if(v->len>1)
		{
			return -1;
		}
		c=v->data[0];
		ret=wtk_char_to_hex(c);
		if(ret==-1){return -1;}
		c=(p->hex1<<4)+ret;
		wtk_strbuf_push_c(buf,c);
		p->sub_state=p->quoted?WTK_STRING2_WAIT_QUOTE_END:WTK_STRING2_WAIT_END;
		break;
	}
	return 0;
}

int wtk_string_parser2_process(wtk_string_parser2_t *p,char *data,int len)
{
	wtk_string_t v;
	char *s,*e;
	int n;
	int ret;
	wtk_strbuf_t *buf;

	buf=wtk_strbuf_new(256,1);
	s=data;
	e=s+len;
	ret=0;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		wtk_string_set(&(v),s,n);
		//wtk_debug("[%.*s]\n",v.len,v.data);
		ret=wtk_string_parse2(p,&v);
		if(ret<0){break;}
//		if(ret!=0)
//		{
//			wtk_debug("[%.*s] ret=%d\n",p->buf->pos,p->buf->data,ret);
//		}
		switch(ret)
		{
		case 0:
			s+=n;
			break;
		case 1:
			wtk_strbuf_push(buf,p->buf->data,p->buf->pos);
			wtk_string_parser2_reset(p);
			s+=n;
			break;
		case 2:
			if(p->buf->pos>0)
			{
				wtk_strbuf_push(buf,p->buf->data,p->buf->pos);
			}else
			{
				wtk_strbuf_push(buf,s,n);
				s+=n;
			}
			wtk_string_parser2_reset(p);
			break;
		default:
			goto end;
			break;
		}
	}
	wtk_strbuf_push(buf,p->buf->data,p->buf->pos);
	wtk_strbuf_reset(p->buf);
	wtk_strbuf_push(p->buf,buf->data,buf->pos);
end:
	wtk_strbuf_delete(buf);
	return ret;
}


void wtk_strkv_parser_test()
{
	wtk_strkv_parser_t psr;
	int ret;
	char *v;
	int v_bytes;

	v="男,k=\"你 好\",xyz=你好 , t = zyt ";
	v_bytes=strlen(v);
	wtk_strkv_parser_init(&(psr),v,v_bytes);
	while(1)
	{
		ret=wtk_strkv_parser_next(&(psr));
		if(ret!=0){break;}
		wtk_debug("[%.*s]=[%.*s]\n",psr.k.len,psr.k.data,psr.v.len,psr.v.data);
		if(wtk_strkv_parser_is_end(&(psr)))
		{
			break;
		}
	}
	//exit(0);
}

void wtk_string_spliter_init(wtk_string_spliter_t *s,char *data,int len,char *sep,int sep_len)
{
	s->s=data;
	s->e=data+len;
	//s->sep=sep;
	wtk_string_set(&(s->sep),sep,sep_len);
	s->state=WTK_STRING_SPLTIER_INIT;
	wtk_string_set(&(s->v),0,0);
}

int wtk_string_spliter_next(wtk_string_spliter_t *s)
{
	int n;
	int ret=0;

	wtk_string_set(&(s->v),0,0);
	s->state=WTK_STRING_SPLTIER_INIT;
	while(s->s<s->e)
	{
		n=wtk_utf8_bytes(*(s->s));
		//wtk_debug("[%.*s]\n",n,s->s);
		switch(s->state)
		{
		case WTK_STRING_SPLTIER_INIT:
			if(wtk_string_cmp(&(s->sep),s->s,n)!=0)
			{
				s->v.data=s->s;
				s->state=WTK_STRING_SPLTIER_WRD;
				if(s->s+n>=s->e)
				{
					s->v.len=s->s+n-s->v.data;
					s->s+=n;
					ret=1;//(s->s+n>=s->e)?1:0;
					goto end;
				}
			}
			break;
		case WTK_STRING_SPLTIER_WRD:
			if(wtk_string_cmp(&(s->sep),s->s,n)==0)
			{
				s->v.len=s->s-s->v.data;
				ret=0;//(s->s+n>=s->e)?1:0;
				s->s+=n;
				goto end;
			}else if(s->s+n>=s->e)
			{
				s->v.len=s->s+n-s->v.data;
				s->s+=n;
				ret=1;//(s->s+n>=s->e)?1:0;
				goto end;
			}
			break;
		}
		s->s+=n;
	}
	ret=-1;
end:
	return ret;
}
