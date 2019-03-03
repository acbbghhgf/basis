#ifndef WTK_SEMDLG_KG_WTK_KGC
#define WTK_SEMDLG_KG_WTK_KGC
#include "wtk/core/wtk_type.h"
#include "wtk/lex/wtk_lex.h"
#include "wtk/core/wtk_os.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_str_parser.h"
#include "wtk/lex/nlg/wtk_nlg2_parser.h"
#include "wtk/core/rbin/wtk_rbin2.h"
#include "wtk_kg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_kgc wtk_kgc_t;

typedef enum
{
	WTK_KGC_INIT,
	WTK_KGC_COMMENT,
	WTK_KGC_VALUE,
}wtk_kgc_state_t;

typedef enum
{
	WTK_KGC_SCOPE_INIT,
	WTK_KGC_SCOPE_CLASS,
	WTK_KGC_SCOPE_ITEM,
	WTK_KGC_SCOPE_INST,
}wtk_kgc_scope_t;

typedef enum
{
	WTK_KGC_ATTR_INIT,
	WTK_KGC_ATTR_ATTR,
	WTK_KGC_ATTR_NEXT,
	WTK_KGC_ATTR_VALUE,
	WTK_KGC_ATTR_FST,
}wtk_kgc_attr_t;

struct wtk_kgc
{
	wtk_heap_t *heap;
	wtk_kg_t *kg;
	wtk_string_parser2_t *strparser;
	wtk_strbuf_t *buf;
	wtk_string_t *pwd;
	wtk_kgc_scope_t scope;
	wtk_kgc_state_t state;
	wtk_kgc_attr_t attr;
	int sub_state;
	wtk_kg_class_t *cls;
	wtk_kg_item_t *item;
	wtk_kg_inst_t *inst;
	wtk_rbin2_t *rbin;
};

wtk_kgc_t* wtk_kgc_new();
void wtk_kgc_delete(wtk_kgc_t *kg);
void wtk_kgc_reset(wtk_kgc_t *kgc);
wtk_kg_t* wtk_kgc_compile(wtk_kgc_t *kg,char *fn);

wtk_kg_t* wtk_kg_new_fn(char *fn,wtk_rbin2_t *rbin);
#ifdef __cplusplus
};
#endif
#endif
