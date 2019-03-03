#ifndef WTK_LEX_NLG_WTK_NLG2_KEY
#define WTK_LEX_NLG_WTK_NLG2_KEY
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_array.h"
#include "wtk/core/wtk_str_parser.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nlg2_key wtk_nlg2_key_t;
/**
 * px(a)
 * ...
 *
 * px(a="b")
 *
 * px(c,b,a)
 *
 *  search px(c,b,a) => section found match
 */

struct wtk_nlg2_key {
    wtk_string_t *k;
    wtk_string_t *v;
};

typedef struct {
    wtk_string_t *name;
    wtk_nlg2_key_t **args;
    int narg;
} wtk_nlg2_function_t;

void wtk_nlg2_function_init(wtk_nlg2_function_t *f);
void wtk_nlg2_function_print(wtk_nlg2_function_t *item);
wtk_string_t* wtk_nlg2_function_get(wtk_nlg2_function_t *item, char *k,
        int k_len);
int wtk_nlg2_function_match(wtk_nlg2_function_t *v1, wtk_nlg2_function_t *v2);
void wtk_nlg2_function_print_key(wtk_nlg2_function_t *item, wtk_strbuf_t *buf);
int wtk_nlg2_function_cmp(wtk_nlg2_function_t *v1, wtk_nlg2_function_t *v2);
float wtk_nlg2_key_cmp(void *ths, wtk_nlg2_key_t **k1, wtk_nlg2_key_t **k2);
void wtk_nlg2_function_init(wtk_nlg2_function_t *f);
int wtk_nlg2_function_parse(wtk_nlg2_function_t *f, wtk_heap_t* heap, char *s,
        int len, int *consume);
#ifdef __cplusplus
}
;
#endif
#endif
