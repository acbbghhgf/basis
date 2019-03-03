#ifndef WTK_LEX_OWL_WTK_OWL
#define WTK_LEX_OWL_WTK_OWL
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_os.h"
#include "wtk_owl_tree.h"
#include "wtk/core/rbin/wtk_rbin2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_owl wtk_owl_t;
#define wtk_owl_check_s(owl,s) wtk_owl_check(owl,s,sizeof(s)-1)
typedef enum {
    WTK_OWL_STATE_INIT, WTK_OWL_STATE_COMMENT1,	//#
    WTK_OWL_STATE_COMMENT2,	// /* ... */
    WTK_OWL_XML_ITEM,
    WTK_OWL_STATE_STRING,
} wtk_owl_state_t;

struct wtk_owl {
    wtk_heap_t *heap;
    wtk_rbin2_t *rbin;
    wtk_owl_state_t state;
    int sub_state;
    int bak_state;
    int bak_sub_state;
    int str_state;
    int str_sub_state;
    wtk_queue_t state_q;
    wtk_strbuf_t *buf;
    wtk_strbuf_t *tmp;
    wtk_owl_tree_t *tree;
    wtk_string_t *pwd;
    wtk_string_t *k;
    unsigned quote :1;
};

wtk_owl_t* wtk_owl_new();
void wtk_owl_delete(wtk_owl_t *owl);
void wtk_owl_check(wtk_owl_t *owl, char *s, int s_bytes);

void wtk_owl_set_state(wtk_owl_t *owl, wtk_owl_state_t state);
int wtk_owl_feed(wtk_owl_t *owl, wtk_string_t *v);
wtk_owl_tree_t* wtk_owl_compile(wtk_owl_t *l, char *data, int bytes);
wtk_owl_tree_t* wtk_owl_compile_file(wtk_owl_t *owl, char *fn);
int wtk_owl_compile_string(wtk_owl_t *l, char *data, int bytes);

wtk_owl_tree_t* wtk_owl_tree_new_fn(char *fn);
wtk_owl_tree_t* wtk_owl_tree_new_fn2(wtk_rbin2_t *rbin, char *fn);
#ifdef __cplusplus
}
;
#endif
#endif
