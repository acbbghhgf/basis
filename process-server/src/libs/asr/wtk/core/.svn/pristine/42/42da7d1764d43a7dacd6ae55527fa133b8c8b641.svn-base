#ifndef WTK_CORE_WTK_STR_PARSER
#define WTK_CORE_WTK_STR_PARSER
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_string_parser wtk_string_parser_t;

struct wtk_string_parser
{
	wtk_strbuf_t *buf;
	int sub_state;
	char quoted_char;
	char hex1;
	unsigned quoted:1;
};

void wtk_string_parser_init(wtk_string_parser_t *p,wtk_strbuf_t *buf);

/**
 * return:
 * 	-1: err
 * 	0: continue
 * 	1: end and input value is consumed
 * 	2: end and input value is not consumed
 */
int wtk_string_parse(wtk_string_parser_t *p,char *data,int bytes);

typedef struct
{
	wtk_strbuf_t *buf;
	int sub_state;
}wtk_var_parse_t;

wtk_var_parse_t* wtk_var_parse_new();
void wtk_var_parse_delete(wtk_var_parse_t *p);
void wtk_var_parse_reset(wtk_var_parse_t *p);
int wtk_var_parse2(wtk_var_parse_t *p,char *data,int bytes);

void wtk_var_parse_init(wtk_var_parse_t *p,wtk_strbuf_t *buf);

/**
 * return:
 * 	-1: err
 * 	0: continue
 * 	1: end and input value is consumed
 */
int wtk_var_parse(wtk_var_parse_t *p,char *data,int bytes);


typedef enum
{
	WTK_STRKV_PARSER_STATE_INIT,
	WTK_STRKV_PARSER_STATE_KEY,
	WTK_STRKV_PARSER_STATE_WAIT_EQ,
	WTK_STRKV_PARSER_STATE_WAIT_VALUE,
	WTK_STRKV_PARSER_STATE_VALUE,
	WTK_STRKV_PARSER_STATE_QUOTE_VALUE,
	WTK_STRKV_PARSER_STATE_WAIT_END,
}wtk_strkv_parser_state_t;

/**
 * a=b,c="d e",
 */
typedef struct
{
	wtk_strkv_parser_state_t state;
	char *s;
	char *e;
	wtk_string_t k;
	wtk_string_t v;
}wtk_strkv_parser_t;

void wtk_strkv_parser_init(wtk_strkv_parser_t *p,char *data,int len);

/**
 *	@return 0 when success
 */
int wtk_strkv_parser_next(wtk_strkv_parser_t *p);
int wtk_strkv_parser_is_end(wtk_strkv_parser_t *p);

typedef wtk_string_t*(*wtk_string_parser2_get_var_f)(void *ths,char *k,int k_len);

typedef struct
{
	wtk_strbuf_t *buf;
	wtk_strbuf_t *var;
	void *ths;
	wtk_string_parser2_get_var_f get_var;
	int sub_state;
	char quoted_char;
	char hex1;
	unsigned quoted:1;
}wtk_string_parser2_t;


wtk_string_parser2_t* wtk_string_parser2_new();
void wtk_string_parser2_delete(wtk_string_parser2_t *p);
void wtk_string_parser2_reset(wtk_string_parser2_t *p);
void wtk_string_parser2_set(wtk_string_parser2_t *p,void *ths,wtk_string_parser2_get_var_f get_var);

/**
 * return:
 * 	-1: err
 * 	0: continue
 * 	1: end and input value is consumed
 * 	2: end and input value is not consumed
 */
int wtk_string_parse2(wtk_string_parser2_t *p,wtk_string_t *v);

int wtk_string_parser2_process(wtk_string_parser2_t *p,char *data,int len);

typedef enum
{
	WTK_STRING_SPLTIER_INIT,
	WTK_STRING_SPLTIER_WRD,
}wtk_string_spliter_state_t;

typedef struct
{
	char *s;
	char *e;
	wtk_string_t sep;
	wtk_string_t v;
	wtk_string_spliter_state_t state;
}wtk_string_spliter_t;

#define wtk_string_spliter_init_s(s,data,len,sep) wtk_string_spliter_init(s,data,len,sep,sizeof(sep)-1)

void wtk_string_spliter_init(wtk_string_spliter_t *s,char *data,int len,char *sep,int sep_len);
int wtk_string_spliter_next(wtk_string_spliter_t *s);
#ifdef __cplusplus
};
#endif
#endif
