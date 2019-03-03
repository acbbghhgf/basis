#ifndef WTK_CORE_JSON_WTK_JSON_PARSE_H_
#define WTK_CORE_JSON_WTK_JSON_PARSE_H_
#include "wtk/core/json/wtk_json.h"
#include "wtk/core/parse/wtk_str_parse.h"
#include "wtk/core/parse/wtk_number_parse.h"
#ifdef __cplusplus
extern "C" {
#endif
//typedef struct wtk_json_parse wtk_json_parse_t;
typedef struct wtk_json_parser wtk_json_parser_t;

typedef enum
{
	WTK_JSON_PARSE_MAIN_WAIT=0,
	WTK_JSON_PARSE_ARRAY,
	WTK_JSON_PARSE_OBJECT,
	WTK_JSON_PARSE_WAIT_VALUE,
	WTK_JSON_PARSE_STRING,
	WTK_JSON_PARSE_NUMBER,
	WTK_JSON_PARSE_NULL,
	WTK_JSON_PARSE_TRUE,
	WTK_JSON_PARSE_FALSE,
}wtk_json_parse_state_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_json_item_t *item;
	wtk_json_item_type_t main_state;
	int sub_state;
}wtk_json_stack_item_t;

struct wtk_json_parser
{
	wtk_json_t *json;
	wtk_json_t *cur_json;
	wtk_json_item_t *cur;
	wtk_json_item_t *value;
	wtk_json_parse_state_t state;
	int sub_state;

	wtk_heap_t *heap;
	wtk_strbuf_t *buf;

	wtk_str_parse_t str_parse;
	wtk_number_parse_t num_parse;
	wtk_queue_t stack_q;
};

wtk_json_parser_t* wtk_json_parser_new();
void wtk_json_parser_delete(wtk_json_parser_t *p);
void wtk_json_parser_reset(wtk_json_parser_t *p);
int wtk_json_parser_parse(wtk_json_parser_t *p,char *data,int len);
int wtk_json_parser_parse2(wtk_json_parser_t *p,wtk_json_t *json,char *data,int len);
int wtk_json_parser_parse_file(wtk_json_parser_t *p,char *fn);
#ifdef __cplusplus
};
#endif
#endif
