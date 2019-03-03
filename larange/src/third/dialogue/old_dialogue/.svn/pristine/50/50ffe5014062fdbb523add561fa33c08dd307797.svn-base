#include "wtk_txtparser_cfg.h"
#include <ctype.h>
/*
===============================================================
	space=[ \r\t\n]
	digit=[0-9]
	alpha=[a-zA-Z]
	purechar = digit | alpha
	extchar=:|.|'|!|-|_
	char=purechar | extchar
	inchar=-|'|:|_
	sep=,|;|?|!|"
	note=space*[tsg]:space*[01]space*
	word=char[inchar|char]*char*(\(note\))*
============================================================
	sent=(space*word[sep|space]+)+
 */
#define USE_NORM
#ifdef USE_NORM
#define is_extchar(c) ((c)==':'||(c)=='.'||(c)=='\''||(c)=='!'||(c)=='-'||(c)=='_')
#define is_char(c) (isalnum(c)||is_extchar(c))
#define is_inchar(c) ((c)=='-'||(c)=='\''||c==':'||c=='_')
#define is_sep(c) ((c)==','||c==';'||c=='?'||c=='!'||c=='\"')
#define is_note(c) ((c)=='s'||c=='t'||c=='g')
#else
#define is_sepchar(c) ((c)==':'||(c)=='.'||(c)=='\''||(c)=='!')
#define is_char(c) (isalnum(c)||is_sepchar(c)||c=='-'||c=='_')
#define is_inchar(c) ((c)=='-'||(c)=='\''||c==':'||c=='_')
#define is_sep(c) ((c)==','||c==';'||c=='?'||c=='!'||c==':')
#define is_note(c) ((c)=='s'||c=='t'||c=='g')
#endif
#define wtk_slot_set_string_s(s,data) wtk_slot_set_string(s,data,sizeof(data)-1)

int wtk_txtparser_cfg_init(wtk_txtparser_cfg_t *cfg)
{
	wtk_string_set_s(&(cfg->extchar),":.'!-_");
	wtk_string_set_s(&(cfg->schar),":.'!-_");
	wtk_string_set_s(&(cfg->inchar),"");
	wtk_string_set_s(&(cfg->sep),",;?!\"");
	wtk_string_set_s(&(cfg->note),"stg");
	cfg->dotwrd=0;
	cfg->dot_hash=0;
	cfg->def_chn_tone=0;
	cfg->use_chn_tone=0;
	cfg->use_utf8=0;
	return 0;
}

int wtk_txtparser_cfg_clean(wtk_txtparser_cfg_t *cfg)
{
	if(cfg->dot_hash)
	{
		wtk_str_hash_delete(cfg->dot_hash);
	}
	return 0;
}

int wtk_txtparser_cfg_update_local(wtk_txtparser_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_b(lc,cfg,use_chn_tone,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,def_chn_tone,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,extchar,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,schar,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,inchar,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,sep,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,note,v);
	cfg->dotwrd=wtk_local_cfg_find_array_s(lc,"dotwrd");

	wtk_local_cfg_update_cfg_b(lc,cfg,use_utf8,v);
	return 0;
}

void wtk_txtparser_cfg_add_dot_word(wtk_txtparser_cfg_t* cfg,char *wrd,int wrd_bytes)
{
	wtk_str_hash_t *h=cfg->dot_hash;
	wtk_string_t *s;
	void *v;

	v=wtk_str_hash_find(h,wrd,wrd_bytes);
	if(v){goto end;}
	s=wtk_heap_dup_string(h->heap,wrd,wrd_bytes);
	wtk_str_hash_add(h,s->data,s->len,s);
end:
	return;
}

void wtk_txtparser_cfg_init_def_dot_hash(wtk_str_hash_t* h)
{
	static wtk_string_t wrd[]={
			wtk_string("co."),
			wtk_string("dr."),
			wtk_string("inc."),
			wtk_string("mr."),
			wtk_string("mrs."),
			wtk_string("r."),
			wtk_string("v."),
			wtk_string("no.")};
	int i,n;

	n=sizeof(wrd)/sizeof(wtk_string_t);
	for(i=0;i<n;++i)
	{
		wtk_str_hash_add(h,wrd[i].data,wrd[i].len,&(wrd[i]));
	}
}

int wtk_txtparser_cfg_update(wtk_txtparser_cfg_t *cfg)
{
	int nslot;

	if(cfg->dotwrd)
	{
		nslot=cfg->dotwrd->nslot*2+1;
	}else
	{
		nslot=13;
	}
	cfg->dot_hash=wtk_str_hash_new(nslot);
	if(cfg->dotwrd)
	{
		wtk_string_t **strs;
		int i;

		strs=(wtk_string_t**)cfg->dotwrd->slot;
		for(i=0;i<cfg->dotwrd->nslot;++i)
		{
			wtk_str_hash_add(cfg->dot_hash,strs[i]->data,strs[i]->len,strs[i]);
		}
	}else
	{
		wtk_txtparser_cfg_init_def_dot_hash(cfg->dot_hash);
	}
	return 0;
}

int wtk_txtparser_cfg_is_extchar(wtk_txtparser_cfg_t *cfg,char c)
{
	//wtk_debug("c=%c,%#x\n",c,c);
	//print_data(cfg->extchar.data,cfg->extchar.len);
	return wtk_string_is_char_in(&(cfg->extchar),c);
}

int wtk_txtparser_cfg_is_schar(wtk_txtparser_cfg_t *cfg,char c)
{
	return (isalnum(c) || wtk_string_is_char_in(&(cfg->schar),c));
}

int wtk_txtparser_cfg_is_char(wtk_txtparser_cfg_t *cfg,char c)
{
	//wtk_debug("isalnum:%d,extchar:%d\n",isalnum(c),wtk_txtparser_cfg_is_extchar(cfg,c));
	//return (isupper(c)|| islower(c) || isdigit(c) ||wtk_txtparser_cfg_is_extchar(cfg,c));
	return (isalnum(c) || wtk_txtparser_cfg_is_extchar(cfg,c));
}

int wtk_txtparser_cfg_is_inchar(wtk_txtparser_cfg_t *cfg,char c)
{
	//wtk_debug("c=%c\n",c);
	return wtk_string_is_char_in(&(cfg->inchar),c);
}

int wtk_txtpaser_cfg_is_sep(wtk_txtparser_cfg_t *cfg,char c)
{
	//wtk_debug("c=%c\n",c);
	return wtk_string_is_char_in(&(cfg->sep),c);
}

int wtk_txtpaser_cfg_is_note(wtk_txtparser_cfg_t *cfg,char c)
{
	return wtk_string_is_char_in(&(cfg->note),c);
}

//============================ charactor, for utf8 =======================
/*
 * @date	2014.06.18
 * @auth	jfyuan
 */
int wtk_string_is_str_in(wtk_string_t* s, wtk_string_t* str)
{
	int ret, i;
	char c = *(str->data);

	ret = wtk_string_is_char_in(s, c);
	for(i=1; i < str->len; i++)
	{
		c = *(str->data + i);
		ret &= wtk_string_is_char_in(s, c);
	}

	return ret;
}

int wtk_txtparser_cfg_is_extchar2(wtk_txtparser_cfg_t *cfg, wtk_string_t* str)
{
	return wtk_string_is_str_in(&(cfg->extchar), str);
}

/* start char of a word */
int wtk_txtparser_cfg_is_schar2(wtk_txtparser_cfg_t *cfg, wtk_string_t* str)
{
	int ret = 0;
	char c;

	if(str->len == 1)
	{
		c = *(str->data);
		ret = (isalnum(c) || wtk_string_is_char_in(&(cfg->schar),c));
	}
	else
	{
		ret = wtk_string_is_str_in(&(cfg->schar), str);
	}

	return ret;
}

int wtk_txtparser_cfg_is_char2(wtk_txtparser_cfg_t *cfg, wtk_string_t* str)
{
	int ret = 0;
	char c;

	if(str->len == 1)
	{
		c = *(str->data);
		ret = (isalnum(c) || wtk_txtparser_cfg_is_extchar(cfg,c));
	}
	else
	{
		ret = wtk_txtparser_cfg_is_extchar2(cfg, str);
	}

	return ret;
}

int wtk_txtparser_cfg_is_inchar2(wtk_txtparser_cfg_t *cfg, wtk_string_t* str)
{
	return wtk_string_is_str_in(&(cfg->inchar), str);
}

int wtk_txtparser_cfg_is_sep2(wtk_txtparser_cfg_t *cfg, wtk_string_t* str)
{
	return wtk_string_is_str_in(&(cfg->sep), str);
}
