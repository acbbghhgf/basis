#ifndef WTK_EVAL_TEXT_WTK_TXTPARSER_H_
#define WTK_EVAL_TEXT_WTK_TXTPARSER_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wtk_array.h"
#include "wtk/core/errno/wtk_eos.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk_txtparser_cfg.h"
#include "wtk/core/wtk_str_encode.h"
#ifdef __cplusplus
extern "C" {
#endif
/*
===============================================================
	space=[ \r\t\n]
	digit=[0-9]
	alpha=[a-zA-Z]
	purechar = digit | alpha
	sepchar=.
	char=purechar | sepchar | -|_
	inchar=-|'|:|_
	sep=,|;|?|!|"
	note=space*[tsg]:space*[01]space*
	word=char[inchar|char]*char*(\(note\))*
============================================================
	sent=(space*word[sep|space]+)+
 */
#define wtk_txtparser_parse_s(p,s) wtk_txtparser_parse(p,s,sizeof(s)-1)
#define wtk_txtparser_set_err_s(p,s) wtk_txtparser_set_err(p,s,sizeof(s)-1)
#define wtk_txtparser_add_dot_word_s(p,w) wtk_txtparser_add_dot_word(p,w,sizeof(w)-1)
typedef struct wtk_txtparser wtk_txtparser_t;
typedef struct wtk_tpword wtk_tpword_t;
typedef int (*wtk_txtparser_is_wrd_f)(void *ths,char *wrd,int wrd_bytes);
typedef int (*wtk_txtparser_wrd_normalize_f)(void *ths,wtk_strbuf_t *wrd);

typedef enum
{
	TP_START,
	TP_WORD,
	TP_INCHAR,
	TP_NOTE_START,
	TP_NOTE_TOK,
	TP_NOTE_SEP,
	TP_NOTE_WAIT_END,
}wtk_tp_state_t;

struct wtk_tpword
{
	wtk_queue_node_t q_n;
	wtk_string_t *name;
	char sep;			//separate char;
	wtk_string_t *sep2;	//separate char, for utf8
	char cur_sense;
	char chn_tone;		//0,1,2,3,4 for chinese tone;
	unsigned s:1;		//stress
	unsigned t:1;		//tone
	unsigned g:1;		//sense group
	unsigned s_set:1;
	unsigned t_set:1;
	unsigned g_set:1;
	unsigned end_sep:1;	//current word have separate char or not;
	unsigned use_usr_pron:1;	//current word has usr pron or not, if 1, use the pron in refText to eval;
};

struct wtk_txtparser
{
	wtk_txtparser_cfg_t *cfg;
	wtk_eos_t *os;
	wtk_tp_state_t state;
	//wtk_str_hash_t *dot_hash;
	wtk_heap_t *heap;
	wtk_strbuf_t *buf;
	wtk_strbuf_t *snt;					//save word as ebnf format for fa, like (sil i want an apple sil)
	wtk_tpword_t *cur;
	wtk_array_t *wrds;					//array of (wtk_tpword_t*)
	wtk_txtparser_is_wrd_f is_wrd_f;
	void *is_wrd_data;
	wtk_txtparser_wrd_normalize_f wrd_normalize_f;
	void *wrd_normalize_ths;
	int char_index;
	unsigned cfg_is_ref:1;			//delete cfg or not when delete parser;
};

wtk_txtparser_t* wtk_txtparser_new(wtk_eos_t *os);
wtk_txtparser_t* wtk_txtparser_new2(wtk_txtparser_cfg_t *cfg,wtk_eos_t *os);
int wtk_txtparser_delete(wtk_txtparser_t *p);
int wtk_txtparser_reset(wtk_txtparser_t *p);
int wtk_txtparser_parse(wtk_txtparser_t *p,char *data,int bytes);
void wtk_txtparser_print(wtk_txtparser_t *p);

//---------------- private section -----------------------------
/**
 * @brief add word to wrds list and also push word to snt buffer for ebnf expand.
 */
void wtk_txtparser_add_word(wtk_txtparser_t *p,char *w,int bytes);

/**
 * @brief add word to wrds list.
 */
wtk_tpword_t* wtk_txtparse_add_tpword(wtk_txtparser_t *p,char *w,int bytes);

/**
 * @brief set word judge;
 */
void wtk_txtparser_set_is_wrd_f(wtk_txtparser_t *p,wtk_txtparser_is_wrd_f is_wrd_f,void *ths);

/**
 * @brief word normalize callback
 */
void wtk_txtparser_set_wrd_normalize_callback(wtk_txtparser_t *p,wtk_txtparser_wrd_normalize_f normalize,void *ths);

/**
 * @brief add dot word
 */
void wtk_txtparser_add_dot_word(wtk_txtparser_t* p,char *wrd,int wrd_bytes);

/**
 * @brief to string;
 */
int wtk_txtparser_to_string(wtk_txtparser_t *p,wtk_strbuf_t *buf,int add_note);

//------------------------ example section --------------------
void wtk_txtparser_test_g();
#ifdef __cplusplus
};
#endif
#endif
