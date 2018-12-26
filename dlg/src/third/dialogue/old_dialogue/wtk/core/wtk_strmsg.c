#include "wtk_strmsg.h"

void wtk_strmsg_init(wtk_strmsg_t *msg,wtk_strbuf_t *buf,int seek)
{
	wtk_strbuf_reset(buf);
	msg->buf=buf;
	msg->state=WTK_STRMSG_INIT;
	msg->len=0;
	msg->seek=seek;
}

void wtk_strmsg_feed_hdr_char(wtk_strmsg_t *msg,char c)
{
	char *p=(char*)&(msg->len);

	switch(msg->state)
	{
	case WTK_STRMSG_INIT:
		if(msg->seek)
		{
			if(c==-1)
			{
				msg->state=WTK_STRMSG_MAGIC0;
			}
		}else
		{
			p[0]=c;
			msg->state=WTK_STRMSG_LEN0;
		}
		break;
	case WTK_STRMSG_MAGIC0:
		if(c==-1)
		{
			msg->state=WTK_STRMSG_MAGIC1;
		}else
		{
			msg->state=WTK_STRMSG_INIT;
		}
		break;
	case WTK_STRMSG_MAGIC1:
		if(c==-1)
		{
			msg->state=WTK_STRMSG_MAGIC2;
		}else
		{
			msg->state=WTK_STRMSG_INIT;
		}
		break;
	case WTK_STRMSG_MAGIC2:
		if(c==-1)
		{
			msg->state=WTK_STRMSG_MAGIC3;
		}else
		{
			msg->state=WTK_STRMSG_INIT;
		}
		break;
	case WTK_STRMSG_MAGIC3:
		p[0]=c;
		msg->state=WTK_STRMSG_LEN0;
		break;
	case WTK_STRMSG_LEN0:
		p[1]=c;
		msg->state=WTK_STRMSG_LEN1;
		break;
	case WTK_STRMSG_LEN1:
		p[2]=c;
		msg->state=WTK_STRMSG_LEN2;
		break;
	case WTK_STRMSG_LEN2:
		p[3]=c;
		if(msg->len==0)
		{
			msg->state=WTK_STRMSG_END;
		}else
		{
			msg->state=WTK_STRMSG_VALUE;
		}
		break;
	default:
		break;
	}
}

int wtk_strmsg_feed(wtk_strmsg_t *msg,char *data,int bytes,int *left)
{
	char *s,*e;
	int ret;
	int n;

	s=data;e=s+bytes;
	if(bytes<=0){ret=-1;goto end;}
	while(msg->state<WTK_STRMSG_VALUE && s<e)
	{
		wtk_strmsg_feed_hdr_char(msg,*s);
		++s;
	}
	if(msg->state==WTK_STRMSG_VALUE && s<e)
	{
		n=e-s;
		n=min(n,msg->len);
		wtk_strbuf_push(msg->buf,s,n);
		s+=n;
		if(msg->buf->pos==msg->len)
		{
			msg->state=WTK_STRMSG_END;
		}
	}
	ret=0;
end:
	if(ret==0 && left)
	{
		*left=e-s;
	}
	return ret;
}

int wtk_strmsg_is_filled(wtk_strmsg_t *msg)
{
	return msg->state==WTK_STRMSG_END;
}
