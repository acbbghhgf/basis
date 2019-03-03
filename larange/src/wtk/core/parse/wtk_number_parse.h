#ifndef WTK_CORE_PARSE_WTK_NUMBER_PARSE_H_
#define WTK_CORE_PARSE_WTK_NUMBER_PARSE_H_
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif
/*
 * 	expr=(e|E)[+|-]*[\d]*
 * 	dec=(0|[1-9]+).[\d]+
 *	[+|-]*(dec|[\d]+)expr*
 *
 *	=>+|-|0|1-9
 */
typedef struct wtk_number_parse wtk_number_parse_t;

typedef enum
{
	WTK_NUMBER_PARSE_WAIT,
	WTK_NUMBER_PARSE_V_WAIT_DOT,
	WTK_NUMBER_PARSE_V_DECIMAL,
	WTK_NUMBER_PARSE_E_FLAG,
	WTK_NUMBER_PARSE_E_VALUE,
	WTK_NUMBER_PARSE_V_VALUE,
}wtk_number_parse_state_t;


struct wtk_number_parse
{
	char state;//wtk_number_parse_state_t;
	int nchar;
	int64_t integer;
	float decimal;
	int e;
	int num_decimal;
	unsigned char vflag:1;		//1=>+;	0=>-
	unsigned char eflag:1;		//1=>+;	0=>-
	unsigned char has_decimal:1;
	unsigned char has_e:1;
};

void wtk_number_parse_init(wtk_number_parse_t *p);
int wtk_number_parse_can_parse(char c);
double wtk_number_parse_to_value(wtk_number_parse_t *p);
int wtk_number_parse_feed(wtk_number_parse_t *p,char c);
double wtk_aton(char *data,int bytes);
void test_number_parse();
#ifdef __cplusplus
};
#endif
#endif
