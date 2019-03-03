#ifndef WTK_CORE_TEXT_WTK_TXTPARSER_CFG_H_
#define WTK_CORE_TEXT_WTK_TXTPARSER_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/core/wtk_slot.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_txtparser_cfg wtk_txtparser_cfg_t;
/*
===============================================================
	space=[ \r\t\n]
	digit=[0-9]
	alpha=[a-zA-Z]
	purechar = digit | alpha
	extchar=:|.|'|!|-|_
    schar=:|.|'|!|-|_
	char=purechar | extchar
	inchar=-|'|:|_
	sep=,|;|?|!|"
	note=space*[tsg]:space*[01]space*
	word=char[inchar|char]*char*(\(note\))*
============================================================
	sent=(space*word[sep|space]+)+
 */

struct wtk_txtparser_cfg
{
	wtk_string_t extchar;
    wtk_string_t schar;     // jfyuan 2013.10.12 add: first char of a word
	wtk_string_t inchar;
	wtk_string_t sep;
	wtk_string_t note;
	wtk_array_t *dotwrd;
	wtk_str_hash_t *dot_hash;
	char def_chn_tone;
	unsigned use_chn_tone:1;
	unsigned use_utf8:1;			// jfyuan 2014.06.10 add: surport text that is encoded by utf8
};

int wtk_txtparser_cfg_init(wtk_txtparser_cfg_t *cfg);
int wtk_txtparser_cfg_clean(wtk_txtparser_cfg_t *cfg);
int wtk_txtparser_cfg_update_local(wtk_txtparser_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_txtparser_cfg_update(wtk_txtparser_cfg_t *cfg);
//------------------------- character -------------------------
void wtk_txtparser_cfg_add_dot_word(wtk_txtparser_cfg_t* cfg,char *wrd,int wrd_bytes);
int wtk_txtparser_cfg_is_extchar(wtk_txtparser_cfg_t *cfg,char c);
int wtk_txtparser_cfg_is_schar(wtk_txtparser_cfg_t *cfg,char c);
int wtk_txtparser_cfg_is_char(wtk_txtparser_cfg_t *cfg,char c);
int wtk_txtparser_cfg_is_inchar(wtk_txtparser_cfg_t *cfg,char c);
int wtk_txtpaser_cfg_is_sep(wtk_txtparser_cfg_t *cfg,char c);
int wtk_txtpaser_cfg_is_note(wtk_txtparser_cfg_t *cfg,char c);
//--------------------- character for utf8 ---------------------
int wtk_txtparser_cfg_is_extchar2(wtk_txtparser_cfg_t *cfg, wtk_string_t* str);
int wtk_txtparser_cfg_is_schar2(wtk_txtparser_cfg_t *cfg, wtk_string_t* str);
int wtk_txtparser_cfg_is_char2(wtk_txtparser_cfg_t *cfg, wtk_string_t* str);
int wtk_txtparser_cfg_is_inchar2(wtk_txtparser_cfg_t *cfg, wtk_string_t* str);
int wtk_txtparser_cfg_is_sep2(wtk_txtparser_cfg_t *cfg, wtk_string_t* str);
#ifdef __cplusplus
};
#endif
#endif
