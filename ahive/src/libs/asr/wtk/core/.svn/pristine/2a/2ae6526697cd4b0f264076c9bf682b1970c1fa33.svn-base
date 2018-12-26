#include "wtk_txtparser.h"
#include <ctype.h>
int wtk_txtparser_feed(wtk_txtparser_t *p,char c);
int wtk_txtparser_feed_start(wtk_txtparser_t *p,char c);
int wtk_txtparser_feed_word(wtk_txtparser_t *p,char c);
int wtk_txtparser_feed_inchar(wtk_txtparser_t *p,char c);
int wtk_txtparser_feed_note_start(wtk_txtparser_t *p,char c);
int wtk_txtparser_feed_note_tok(wtk_txtparser_t *p,char c);
int wtk_txtparser_feed_note_sep(wtk_txtparser_t *p,char c);

int wtk_txtparser_feed2(wtk_txtparser_t *p, wtk_string_t *str);
int wtk_txtparser_feed_start2(wtk_txtparser_t *p, wtk_string_t *str);
int wtk_txtparser_feed_word2(wtk_txtparser_t *p, wtk_string_t *str);
int wtk_txtparser_feed_inchar2(wtk_txtparser_t *p, wtk_string_t *str);

void wtk_tpword_print(wtk_tpword_t *w)
{
	printf("%*.*s\n",w->name->len,w->name->len,w->name->data);
	printf("s:\t%d\n",w->s);
	printf("t:\t%d\n",w->t);
	printf("g:\t%d\n",w->g);
	printf("chntone:\t%d\n",w->chn_tone);
	if(w->sep)
	{
		printf("sep:\t%c\n",w->sep);
	}
	if(w->sep2)
	{
		printf("sep2:\t%.*s\n",w->sep2->len,w->sep2->data);
	}
}


void wtk_txtparser_init(wtk_txtparser_t *p,wtk_eos_t *os,wtk_txtparser_cfg_t *cfg,int cfg_is_ref)
{
	p->cfg=cfg;
	p->cfg_is_ref=cfg_is_ref;
	p->os=os;
	p->heap=wtk_heap_new(4096);
	p->buf=wtk_strbuf_new(64,1);
	p->snt=wtk_strbuf_new(4096,1);
	//p->dot_hash=wtk_str_hash_new(337);
	p->is_wrd_f=0;
	p->is_wrd_data=0;
	p->wrd_normalize_f=0;
	p->wrd_normalize_ths=0;
	//wtk_txtparser_init_dot_hash(p);
	wtk_txtparser_reset(p);
}

wtk_txtparser_t* wtk_txtparser_new(wtk_eos_t *os)
{
	wtk_txtparser_t *p;
	wtk_txtparser_cfg_t *cfg;

	cfg=(wtk_txtparser_cfg_t*)wtk_malloc(sizeof(*cfg));
	wtk_txtparser_cfg_init(cfg);
	wtk_txtparser_cfg_update(cfg);
	p=(wtk_txtparser_t*)wtk_malloc(sizeof(*p));
	wtk_txtparser_init(p,os,cfg,0);
	return p;
}

wtk_txtparser_t* wtk_txtparser_new2(wtk_txtparser_cfg_t *cfg,wtk_eos_t *os)
{
	wtk_txtparser_t *p;

	p=(wtk_txtparser_t*)wtk_malloc(sizeof(*p));
	wtk_txtparser_init(p,os,cfg,1);
	return p;
}

int wtk_txtparser_delete(wtk_txtparser_t *p)
{
	if(!p->cfg_is_ref)
	{
		wtk_txtparser_cfg_clean(p->cfg);
		wtk_free(p->cfg);
	}
	//wtk_str_hash_delete(p->dot_hash);
	wtk_strbuf_delete(p->buf);
	wtk_strbuf_delete(p->snt);
	wtk_heap_delete(p->heap);
	wtk_free(p);
	return 0;
}

void wtk_txtparser_set_is_wrd_f(wtk_txtparser_t *p,wtk_txtparser_is_wrd_f is_wrd_f,void *is_wrd_data)
{
	p->is_wrd_f=is_wrd_f;
	p->is_wrd_data=is_wrd_data;
}

void wtk_txtparser_set_wrd_normalize_callback(wtk_txtparser_t *p,wtk_txtparser_wrd_normalize_f normalize,void *ths)
{
	p->wrd_normalize_f=normalize;
	p->wrd_normalize_ths=ths;
}

int wtk_txtparser_reset(wtk_txtparser_t *p)
{
	wtk_heap_reset(p->heap);
	p->wrds=wtk_array_new_h(p->heap,1024,sizeof(wtk_tpword_t*));
	wtk_strbuf_reset(p->buf);
	p->cur=0;
	p->state=TP_START;
	return 0;
}

void wtk_txtparser_add_dot_word(wtk_txtparser_t* p,char *wrd,int wrd_bytes)
{
	wtk_txtparser_cfg_add_dot_word(p->cfg,wrd,wrd_bytes);
}

wtk_tpword_t* wtk_txtparser_new_word(wtk_txtparser_t *p)
{
	wtk_tpword_t *w;

	w=(wtk_tpword_t*)wtk_heap_malloc(p->heap,sizeof(*w));
	w->name=0;
	w->s=w->t=w->g=0;
	w->s_set=w->t_set=w->g_set=0;
	w->end_sep=0;w->sep=0;
	w->use_usr_pron=0;
	w->chn_tone=0;
	w->sep2 = 0;
	return w;
}

int wtk_txtparser_is_wrd(wtk_txtparser_t *p,char *wrd,int wrd_bytes)
{
	return p->is_wrd_f?p->is_wrd_f(p->is_wrd_data,wrd,wrd_bytes):0;
}

int wtk_txtparser_normal_buf(wtk_txtparser_t* p,wtk_strbuf_t *buf)
{
	int found;
	char c;
	wtk_string_t *v;

	found=wtk_txtparser_is_wrd(p,buf->data,buf->pos);
	if(found){goto end;}
	c=buf->data[buf->pos-1];
	if(c=='.')
	{
		v=wtk_str_hash_find(p->cfg->dot_hash,buf->data,buf->pos);
		found=v?1:0;
		if(!found)
		{
			--buf->pos;
			p->cur->sep='.';
		}

        /*------- jfyuan 20121012 add:start -------- */
        /* deal with unnormal case, such as smoking'. */
        c = buf->data[buf->pos-1];
        if(c=='\'' || c=='-') --buf->pos;
        /*------- jfyuan 20121012 add:end -------- */
	}else if(wtk_txtparser_cfg_is_extchar(p->cfg,c))
	{
		--buf->pos;
		p->cur->sep=c;
	}
end:
	return 0;
}

void wtk_txtparser_set_err(wtk_txtparser_t *p,char *msg,int msg_bytes)
{
	if(p->buf->pos>0)
	{
		wtk_strbuf_push_s(p->buf,": ");
	}
	wtk_strbuf_push(p->buf,msg,msg_bytes);
	wtk_strbuf_push_f(p->buf," (index: %d).",p->char_index);
	if(p->os)
	{
		wtk_errno_set(p->os->err,WTK_EVAL_REF_INVALID,p->buf->data,p->buf->pos);
	}else
	{
		wtk_debug("%*.*s\n",p->buf->pos,p->buf->pos,p->buf->data);
	}
}

int wtk_txtparser_peek_word(wtk_txtparser_t *p)
{
	wtk_strbuf_t *buf=p->buf;
	wtk_tpword_t *w=p->cur;
	int ret=-1;

	//wtk_debug("pos=%d\n",buf->pos);
	//print_data(buf->data,buf->pos);
	if(!w||buf->pos<=0){goto end;}
	wtk_txtparser_normal_buf(p,buf);
	//wtk_debug("%*.*s\n",buf->pos,buf->pos,buf->data);
	if(p->cfg->use_chn_tone)
	{
		char t;

		t=buf->data[buf->pos-1];
		//wtk_debug("tone: %c\n",t);
		if(isdigit(t))
		{
			if(t>'0' && t<='4')
			{
				w->chn_tone=t-'0';
			}
			--buf->pos;
		}
	}
	if(p->wrd_normalize_f)
	{
		p->wrd_normalize_f(p->wrd_normalize_ths,buf);
	}
	if(buf->pos<=0){ret=0;goto end;}
	//wtk_debug("buf->pos=%d\n",buf->pos);
	w->name=wtk_heap_dup_string(p->heap,buf->data,buf->pos);
	if(p->wrds->nslot>0)
	{
		wtk_strbuf_push_c(p->snt,' ');
	}
	wtk_strbuf_push(p->snt,buf->data,buf->pos);
	//wtk_debug("%p:g=%d\n",w,w->g);
	//wtk_tpword_print(w);
	((wtk_tpword_t **)wtk_array_push(p->wrds))[0]=w;
	p->cur=0;ret=0;
	//wtk_tpword_print(w);
end:
	return ret;
}

int wtk_txtparser_feed_note_wait_end(wtk_txtparser_t *p,char c)
{
	int ret=0;

	if(c==')')
	{
		p->state=TP_WORD;
	}else if(c==',')
	{
		p->state=TP_NOTE_START;
	}else if(!isspace(c))
	{
		wtk_txtparser_set_err_s(p,"invalid char in sense end");
		ret=-1;
	}
	return ret;
}

int wtk_txtparser_feed_note_sep(wtk_txtparser_t *p,char c)
{
	wtk_tpword_t *w;
	int ret=0,v;

	if(c=='0' || c=='1')
	{
		w=p->cur;
		v=c-'0';
		switch(w->cur_sense)
		{
		case 's':
			w->s=v;
			w->s_set=1;
			break;
		case 't':
			w->t=v;
			w->t_set=1;
			break;
		case 'g':
			w->g=v;
			w->g_set=1;
			break;
		}
		p->state=TP_NOTE_WAIT_END;
	}else if(!isspace(c))
	{
		wtk_txtparser_set_err_s(p,"invalid char in sense tok value");
		ret=-1;
	}
	return ret;
}

int wtk_txtparser_feed_note_start(wtk_txtparser_t *p,char c)
{
	wtk_tpword_t *w;
	int ret=0;

	if(wtk_txtpaser_cfg_is_note(p->cfg,c))
	{
		w=p->cur;
		w->cur_sense=c;
		p->state=TP_NOTE_TOK;
	}else if(!isspace(c))
	{
		wtk_txtparser_set_err_s(p,"invalid char in sense tok");
		ret=-1;
	}
	return ret;
}

int wtk_txtparser_feed_note_tok(wtk_txtparser_t *p,char c)
{
	int ret=0;

	if(c==':')
	{
		p->state=TP_NOTE_SEP;
	}else if(!isspace(c))
	{
		wtk_txtparser_set_err_s(p,"invalid char in sense tok sep");
		ret=-1;
	}
	return ret;
}

int wtk_txtparser_feed_start(wtk_txtparser_t *p,char c)
{
	if(wtk_txtparser_cfg_is_schar(p->cfg,c))
	{
		p->state=TP_WORD;
		p->cur=wtk_txtparser_new_word(p);
		wtk_strbuf_reset(p->buf);
		wtk_strbuf_push_c(p->buf,c);
	}
	return 0;
}

int wtk_txtparser_feed_word(wtk_txtparser_t *p,char c)
{
	wtk_strbuf_t* buf=p->buf;
	int ret=0;

	if(wtk_txtparser_cfg_is_char(p->cfg,c))
	{
		wtk_strbuf_push_c(buf,c);
	}else if(wtk_txtparser_cfg_is_inchar(p->cfg,c))
	{
		wtk_strbuf_push_c(buf,c);
		p->state=TP_INCHAR;
	}else if(c=='(')
	{
		p->state=TP_NOTE_START;
	}else if(isspace(c)||c==-1)
	{
		ret=wtk_txtparser_peek_word(p);
		p->state=TP_START;
	}else if(wtk_txtpaser_cfg_is_sep(p->cfg,c))
	{
		p->cur->end_sep=1;
		p->cur->sep=c;
		ret=wtk_txtparser_peek_word(p);
		p->state=TP_START;
	}else
	{
		ret=wtk_txtparser_peek_word(p);
		p->state=TP_START;
		//wtk_txtparser_set_err_s(p," word is end by non-char.");
		//ret=-1;
	}
	return ret;
}

int wtk_txtparser_feed_inchar(wtk_txtparser_t *p,char c)
{
	wtk_strbuf_t* buf=p->buf;
	int ret=0;

	if(wtk_txtparser_cfg_is_char(p->cfg,c))
	{
		wtk_strbuf_push_c(buf,c);
		p->state=TP_WORD;
	}else if(wtk_txtparser_cfg_is_inchar(p->cfg,c))
	{
		wtk_strbuf_push_c(buf,c);
    }else
	{
		wtk_txtparser_set_err_s(p,"word is end by in-char");
		ret=-1;
	}
	return ret;
}

int wtk_txtparser_feed(wtk_txtparser_t *p,char c)
{
	int ret;

	//printf("%c,%d\n",c,p->state);
	switch(p->state)
	{
	case TP_START:
		ret=wtk_txtparser_feed_start(p,c);
		break;
	case TP_WORD:
		ret=wtk_txtparser_feed_word(p,c);
		break;
	case TP_INCHAR:
		ret=wtk_txtparser_feed_inchar(p,c);
		break;
	case TP_NOTE_START:
		ret=wtk_txtparser_feed_note_start(p,c);
		break;
	case TP_NOTE_TOK:
		ret=wtk_txtparser_feed_note_tok(p,c);
		break;
	case TP_NOTE_SEP:
		ret=wtk_txtparser_feed_note_sep(p,c);
		break;
	case TP_NOTE_WAIT_END:
		ret=wtk_txtparser_feed_note_wait_end(p,c);
		break;
	default:
		wtk_txtparser_set_err_s(p,"unexpected state");
		ret=-1;
		break;
	}
	return ret;
}

void wtk_txtparser_post_process(wtk_txtparser_t *tp)
{
	wtk_tpword_t **wrds,*wrd;

	if(tp->wrds->nslot<=0){goto end;}
	wrds=(wtk_tpword_t**)tp->wrds->slot;
	wrd=wrds[tp->wrds->nslot-1];
	if(!wrd->g_set)
	{
		wrd->g=1;
	}
end:
	return;
}

int wtk_txtparser_parse(wtk_txtparser_t *tp,char *data,int bytes)
{
	wtk_strbuf_t *snt=tp->snt;
	char *p=data,*end=data+bytes;
	int ret=-1;

	//print_data(data,bytes);
	tp->char_index=0;
	wtk_strbuf_reset(snt);
	wtk_strbuf_push_s(snt,"(sil ");

	if(tp->cfg->use_utf8)
	{
		int cnt;
		wtk_string_t v;

		while(p < end)
		{
			cnt = wtk_utf8_bytes(*p);
			wtk_string_set(&(v), p, cnt);
			//wtk_debug("v = %.*s[%d]\n", v.len, v.data, v.len);
			ret = wtk_txtparser_feed2(tp, &v);
			if(0 != ret) { goto end; }

			p += cnt;
		}
		ret = wtk_txtparser_feed(tp, -1);
		if(0 != ret) { goto end; }
		ret=tp->wrds->nslot>0?0:-1;
		if(ret!=0){goto end;}
	}
	else
	{
		while(p<end)
		{
			ret=wtk_txtparser_feed(tp,tolower(*p));
			if(ret!=0){goto end;}
			++tp->char_index;
			++p;
		}
		ret=wtk_txtparser_feed(tp,-1);
		if(ret!=0){goto end;}
		ret=tp->wrds->nslot>0?0:-1;
		if(ret!=0){goto end;}
		wtk_txtparser_post_process(tp);
	}

	wtk_strbuf_push_s(snt," sil)");
end:
	return ret;
}

wtk_tpword_t* wtk_txtparse_add_tpword(wtk_txtparser_t *p,char *w,int bytes)
{
	wtk_tpword_t *tw;
	wtk_heap_t *heap=p->heap;

	tw=wtk_txtparser_new_word(p);
	tw->name=wtk_heap_dup_string(heap,w,bytes);
	((wtk_tpword_t **)wtk_array_push(p->wrds))[0]=tw;
	return tw;
}

void wtk_txtparser_add_word(wtk_txtparser_t *p,char *w,int bytes)
{
	wtk_txtparse_add_tpword(p,w,bytes);
	if(p->wrds->nslot>0)
	{
		wtk_strbuf_push_c(p->snt,' ');
	}
	wtk_strbuf_push(p->snt,w,bytes);
}

int wtk_txtparser_to_string(wtk_txtparser_t *p,wtk_strbuf_t *buf,int add_note)
{
	wtk_tpword_t **wrds;
	wtk_tpword_t *wrd;
	int i,nwrd;
	int n;

	wtk_strbuf_reset(buf);
	if(!p->wrds){nwrd=0;goto end;}
	wrds=(wtk_tpword_t**)p->wrds->slot;
	//wtk_debug("nslot: %d\n",p->wrds->nslot);
	nwrd=p->wrds->nslot;
	for(i=0;i<nwrd;++i)
	{
		wrd=wrds[i];
		if(i>0)
		{
			wtk_strbuf_push_s(buf," ");
		}
		wtk_strbuf_push(buf,wrd->name->data,wrd->name->len);
		if(add_note && (wrd->t || wrd->g ||wrd->s))
		{
			n=0;
			wtk_strbuf_push_s(buf,"(");
			if(wrd->t)
			{
				wtk_strbuf_push_s(buf,"t:1");
				++n;
			}
			if(wrd->g)
			{
				if(n>0)
				{
					wtk_strbuf_push_s(buf,",");
				}
				wtk_strbuf_push_s(buf,"g:1");
				++n;
			}
			if(wrd->s)
			{
				if(n>0)
				{
					wtk_strbuf_push_s(buf,",");
				}
				wtk_strbuf_push_s(buf,"s:1");
				++n;
			}
			wtk_strbuf_push_s(buf,")");
		}
	}
end:
	return nwrd;
}

void wtk_txtparser_print(wtk_txtparser_t *p)
{
	wtk_tpword_t **wrds;
	int i;

	printf("%*.*s\n",p->snt->pos,p->snt->pos,p->snt->data);
	wrds=(wtk_tpword_t**)p->wrds->slot;
	for(i=0;i<p->wrds->nslot;++i)
	{
		wtk_tpword_print(wrds[i]);
	}
}

//------------------ surport utf8 ---------------
/*
 * @auth:	jfyuan
 * @date:	2014-06-10
 * @brief:	surport the text which is encoded by utf-8
 * @see
 */

/* start word */
int wtk_txtparser_feed_start2(wtk_txtparser_t *p, wtk_string_t *str)
{
	p->state = TP_WORD;
	p->cur = wtk_txtparser_new_word(p);
	wtk_strbuf_reset(p->buf);
	wtk_strbuf_push(p->buf, str->data, str->len);

	return 0;
}

/* in char */
int wtk_txtparser_feed_inchar2(wtk_txtparser_t *p, wtk_string_t *str)
{
	wtk_strbuf_t* buf=p->buf;
	int ret=0;

	if(wtk_txtparser_cfg_is_char2(p->cfg, str))
	{
		wtk_strbuf_push(buf, str->data, str->len);
		p->state=TP_WORD;
	}else if(wtk_txtparser_cfg_is_inchar2(p->cfg, str))
	{
		wtk_strbuf_push(buf, str->data, str->len);
    }else
	{
		wtk_txtparser_set_err_s(p,"word is end by in-char");
		ret=-1;
	}
	return ret;
}

int wtk_txtparser_feed_word2(wtk_txtparser_t *p, wtk_string_t *str)
{
	wtk_strbuf_t* buf=p->buf;
	char c;
	int ret=0;

	if(str->len == 1)
	{
		c = *(str->data);

		if(wtk_txtparser_cfg_is_inchar(p->cfg,c))
		{
			wtk_strbuf_push_c(buf,c);
			p->state=TP_INCHAR;
		}else if(c=='(')
		{
			p->state=TP_NOTE_START;
		}else if(isspace(c)||c==-1)
		{
			ret=wtk_txtparser_peek_word(p);
			p->state=TP_START;
		}else if(wtk_txtpaser_cfg_is_sep(p->cfg,c))
		{
			p->cur->end_sep=1;
			//p->cur->sep=c;
			p->cur->sep2 = wtk_heap_dup_string(p->heap, str->data, str->len);
			ret=wtk_txtparser_peek_word(p);
			p->state=TP_START;
		}else
		{
			ret=wtk_txtparser_peek_word(p);
			p->state=TP_START;
		}
	}
	else
	{
		if(wtk_txtparser_cfg_is_inchar2(p->cfg, str))
		{
			wtk_strbuf_push(buf, str->data, str->len);
			p->state=TP_INCHAR;
		}
		else if(wtk_txtparser_cfg_is_sep2(p->cfg, str))
		{
			p->cur->end_sep = 1;
			p->cur->sep2 = wtk_heap_dup_string(p->heap, str->data, str->len);
			ret = wtk_txtparser_peek_word(p);
			p->state = TP_START;
			goto end;
		}

		wtk_strbuf_push(buf, str->data, str->len);
	}

end:

	return ret;
}

int wtk_txtparser_feed2(wtk_txtparser_t *p, wtk_string_t *str)
{
	int ret;
	char c = *(str->data);

	//printf("%c,%d\n",c,p->state);
	switch(p->state)
	{
	case TP_START:
		ret = wtk_txtparser_feed_start2(p, str);
		break;
	case TP_WORD:
		ret = wtk_txtparser_feed_word2(p, str);
		break;
	case TP_INCHAR:
		ret = wtk_txtparser_feed_inchar2(p, str);
		break;
	case TP_NOTE_START:
		ret = wtk_txtparser_feed_note_start(p, c);
		break;
	case TP_NOTE_TOK:
		ret=wtk_txtparser_feed_note_tok(p,c);
		break;
	case TP_NOTE_SEP:
		ret=wtk_txtparser_feed_note_sep(p,c);
		break;
	case TP_NOTE_WAIT_END:
		ret=wtk_txtparser_feed_note_wait_end(p,c);
		break;
	default:
		wtk_txtparser_set_err_s(p,"unexpected state");
		ret=-1;
		break;
	}
	return ret;
}

//------------------------ example section --------------------
void wtk_txtparser_test_g()
{
	wtk_txtparser_t *p;
	int i;

	p=wtk_txtparser_new(0);
	for(i=0;i<10;++i)
	{
		wtk_txtparser_parse_s(p,"yes hello. world-check me");
		wtk_txtparser_print(p);
		wtk_txtparser_reset(p);
	}
	wtk_txtparser_delete(p);
}
