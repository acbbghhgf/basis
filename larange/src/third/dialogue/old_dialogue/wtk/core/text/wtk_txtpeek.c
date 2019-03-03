#include "wtk_txtpeek.h" 
#include "wtk/core/wtk_str_encode.h"
#include <ctype.h>

wtk_txtpeek_t* wtk_txtpeek_new(wtk_txtpeek_cfg_t *cfg)
{
	wtk_txtpeek_t *tp;

	tp=(wtk_txtpeek_t*)wtk_malloc(sizeof(wtk_txtpeek_t));
	tp->cfg=cfg;
	tp->s=NULL;
	tp->e=NULL;
	return tp;
}

void wtk_txtpeek_delete(wtk_txtpeek_t *p)
{
	wtk_free(p);
}

void wtk_txtpeek_set(wtk_txtpeek_t *p,char *txt,int bytes)
{
	p->s=txt;
	p->e=p->s+bytes;
}

wtk_txtpeek_item_t wtk_txtpeek_next(wtk_txtpeek_t *p)
{
typedef enum
{
	WTK_TXTPEEK_INIT_STATE,
	WTK_TXTPEEK_NUM_STATE,
	WTK_TXTPEEK_ENG_STATE,
}wtk_txtpeek_state_t;
	wtk_txtpeek_item_t v = {0};
	wtk_txtpeek_state_t state;
	char c;
	int cnt;

	wtk_string_set(&(v.v),0,0);
	state=WTK_TXTPEEK_INIT_STATE;
	//wtk_debug("[%p/%p]\n",p->s,p->e);
	while(p->s<p->e)
	{
		c=*(p->s);
		cnt=wtk_utf8_bytes(c);
		//wtk_debug("v[%d]=[%.*s]\n",cnt,cnt,p->s);
		switch(state)
		{
		case WTK_TXTPEEK_INIT_STATE:
			if(cnt>1)
			{
				v.v.data=p->s;
				v.v.len=cnt;
				//wtk_debug("[%.*s]\n",v.v.len,v.v.data);
				v.type=WTK_TXTPEEK_CHN;
				p->s+=cnt;
				goto end;
			}else
			{
				if(isdigit(c))
				{
					v.v.data=p->s;
					v.type=WTK_TXTPEEK_NUM;
					if(p->cfg->merge_num)
					{
						state=WTK_TXTPEEK_NUM_STATE;
					}else
					{
						v.v.len=cnt;
						p->s+=cnt;
						goto end;
					}
				}else if(isalpha(c))
				{
					v.v.data=p->s;
					v.type=WTK_TXTPEEK_ENG;
					state=WTK_TXTPEEK_ENG_STATE;
				}
			}
			break;
		case WTK_TXTPEEK_NUM_STATE:
			if(cnt!=1 || !isdigit(c))
			{
				v.v.len=p->s-v.v.data;
				goto end;
			}else if(p->s+cnt>=p->e)
			{
				v.v.len=p->s+cnt-v.v.data;
				goto end;
			}
			break;
		case WTK_TXTPEEK_ENG_STATE:
			if(cnt!=1 || !isalpha(c))
			{
				v.v.len=p->s-v.v.data;
				goto end;
			}else if(p->s+cnt>=p->e)
			{
				v.v.len=p->s+cnt-v.v.data;
				goto end;
			}
			break;
		}
		p->s+=cnt;
	}
	v.type=WTK_TXTPEEK_EOF;
end:
	//wtk_debug("type=%d\n",v.type);
	return v;
}

void wtk_txtpeek_process(wtk_txtpeek_t *p,char *txt,int bytes,wtk_strbuf_t *buf)
{
	wtk_txtpeek_item_t item;

	wtk_strbuf_reset(buf);
	wtk_txtpeek_set(p,txt,bytes);
	while(1)
	{
		item=wtk_txtpeek_next(p);
		//wtk_debug("v[%d]=[%.*s] type=%d\n",item.v.len,item.v.len,item.v.data,item.type);
		if(item.type==WTK_TXTPEEK_EOF)
		{
			break;
		}
		wtk_strbuf_push(buf,item.v.data,item.v.len);
	}
}
