#ifndef WTK_LEX_NLG_WTK_NLG2_PARSER
#define WTK_LEX_NLG_WTK_NLG2_PARSER
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_os.h"
#include "wtk/core/wtk_str_parser.h"
#include "wtk/core/wtk_larray.h"
#include "wtk_nlg2.h"
#include "wtk_rexpr_parser.h"
#include "wtk/core/rbin/wtk_rbin2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nlg2_parser wtk_nlg2_parser_t;

typedef enum {
    WTK_NLG2_INIT = 0,
    WTK_NLG2_COMMENT,
    WTK_NLG2_INCLUDE,
    WTK_NLG2_ACTION,
    WTK_NLG2_ITEM,
    WTK_NLG2_ITEM_EXPR,
    WTK_NLG2_ITEM_LUA,
} wtk_nlg2_state_t;

struct wtk_nlg2_parser {
    wtk_heap_t *heap;
    wtk_strbuf_t *buf;
    wtk_string_t *pwd;
    wtk_nlg2_t *nlg;
    wtk_nlg2_state_t state;
    int sub_state;
    wtk_string_parser2_t *strparser;
    wtk_nlg2_item_t *item;
    wtk_larray_t *args;
    wtk_rexpr_parser_t *re_parser;
};

wtk_nlg2_parser_t* wtk_nlg2_parser_new();
void wtk_nlg2_parser_delete(wtk_nlg2_parser_t *p);
void wtk_nlg2_parser_reset(wtk_nlg2_parser_t *p);
wtk_nlg2_t* wtk_nlg2_paser_load(wtk_nlg2_parser_t *p, char *fn,
        wtk_rbin2_t *rbin);
wtk_nlg2_t* wtk_nlg2_new_fn(char *fn, wtk_rbin2_t *rbin);
wtk_nlg2_t* wtk_nlg2_new_str(char *data, int len);
#ifdef __cplusplus
}
;
#endif
#endif
