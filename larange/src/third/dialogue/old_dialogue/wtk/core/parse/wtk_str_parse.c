#include "wtk_str_parse.h"

void wtk_str_parse_init(wtk_str_parse_t *p,wtk_strbuf_t *buf)
{
	p->buf=buf;
	wtk_strbuf_reset(buf);
	p->esc=0;
	p->v=0;
	p->state=WTK_STR_PARSE_NORMAL;
}


int wtk_str_parse_feed(wtk_str_parse_t *p,char c)
{
	wtk_strbuf_t *buf=p->buf;
	int v;
	char tmp[3];
	int len;

	//wtk_debug("[%c] %d\n",c,p->esc);
	if(p->state==WTK_STR_PARSE_NORMAL)
	{
		if(p->esc)
		{
			switch(c)
			{
			case 'b':
				wtk_strbuf_push_c(buf,'\b');
				break;
			case 'f':
				wtk_strbuf_push_c(buf,'\f');
				break;
			case 'n':
				wtk_strbuf_push_c(buf,'\n');
				break;
			case 'r':
				wtk_strbuf_push_c(buf,'\r');
				break;
			case 't':
				wtk_strbuf_push_c(buf,'\t');
				break;
			case 'u':
				p->state=WTK_STR_PARSE_U0;
				break;
			case 'x':
				p->state=WTK_STR_PARSE_H0;
				break;
			default:
				wtk_strbuf_push_c(buf,c);
				break;
			}
			p->esc=0;
		}else
		{
			if(c=='\"')
			{
				//wtk_debug("[%.*s],esc=%d\n",buf->pos,buf->data,p->esc);
				return 1;
			}else if(c=='\\')
			{
				p->esc=1;
			}else
			{
				wtk_strbuf_push_c(buf,c);
			}
		}
	}else
	{
		if(c>='a')
		{
			v=c-'a'+10;
		}else if(c>='A')
		{
			v=c-'A'+10;
		}else if(c>='0')
		{
			v=c-'0';
		}else
		{
			if(p->state==WTK_STR_PARSE_H1)
			{
				p->state=WTK_STR_PARSE_NORMAL;
				wtk_strbuf_push_c(buf,(char)p->v);
				return 0;
			}else
			{
				return -1;
			}
		}
		switch(p->state)
		{
		case WTK_STR_PARSE_U0:
			p->v=v;
			p->state=WTK_STR_PARSE_U1;
			break;
		case WTK_STR_PARSE_U1:
			p->v=(p->v<<4)+v;
			p->state=WTK_STR_PARSE_U2;
			break;
		case WTK_STR_PARSE_U2:
			p->v=(p->v<<4)+v;
			p->state=WTK_STR_PARSE_U3;
			break;
		case WTK_STR_PARSE_U3:
			p->v=(p->v<<4)+v;
			p->state=WTK_STR_PARSE_NORMAL;
			//wtk_debug("[%#x]\n",p->v);
			len=wtk_utf16_to_utf8(p->v,tmp);
			if(len>0)
			{
				wtk_strbuf_push(buf,tmp,len);
			}
			break;
		case WTK_STR_PARSE_H0:
			p->v=v;
			p->state=WTK_STR_PARSE_H1;
			break;
		case WTK_STR_PARSE_H1:
			p->v=(p->v<<4)+v;
			p->state=WTK_STR_PARSE_NORMAL;
			wtk_strbuf_push_c(buf,(char)p->v);
			break;

		}
	}
	return 0;
}


//----------------------- test str parse ---------------------
void test_str_parse()
{
	wtk_str_parse_t parse;
	wtk_strbuf_t *buf;
	char *p="\x31\xa你好我在abc\\u4f60\\u597d\\\"\"";
	char *s,*e;
	int ret;

	buf=wtk_strbuf_new(256,1);
	wtk_str_parse_init(&(parse),buf);
	s=p;e=p+strlen(p);
	while(s<e)
	{
		ret=wtk_str_parse_feed(&parse,*s);
		if(ret!=0)
		{
			break;
		}
		++s;
	}
	wtk_debug("[%.*s]\n",buf->pos,buf->data);
}
