#ifndef WTK_CORE_PARSE_WTK_STR_PARSE_H_
#define WTK_CORE_PARSE_WTK_STR_PARSE_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * use in json,stop when found '\"'
 */
typedef struct wtk_str_parse wtk_str_parse_t;

typedef enum
{
	WTK_STR_PARSE_NORMAL,
	WTK_STR_PARSE_U0,
	WTK_STR_PARSE_U1,
	WTK_STR_PARSE_U2,
	WTK_STR_PARSE_U3,
	WTK_STR_PARSE_H0,
	WTK_STR_PARSE_H1,
	WTK_STR_PARSE_D0,
	WTK_STR_PARSE_D1,
}wtk_str_parse_xx_t;

/*
	\"
	\\
	\/
	\b
	\f
	\n
	\r
	\t
	\u four-hex-digits
	\ddd
	\xhh
 */
struct wtk_str_parse
{
	wtk_strbuf_t *buf;
	int v;
	char	state;	//wtk_str_parse_xx_t
	unsigned esc:1;
};

void wtk_str_parse_init(wtk_str_parse_t *p,wtk_strbuf_t *buf);

/**
 *	@return:
 *			0, continue;
 *			1, string end,found '\"';
 *			-1,	failed;
 */
int wtk_str_parse_feed(wtk_str_parse_t *p,char c);


//----------------------- test str parse ---------------------
void test_str_parse();
#ifdef __cplusplus
};
#endif
#endif
