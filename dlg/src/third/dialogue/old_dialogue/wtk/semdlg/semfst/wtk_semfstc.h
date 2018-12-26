#ifndef WTK_SEMDLG_SEMFST_WTK_SEMFSTC
#define WTK_SEMDLG_SEMFST_WTK_SEMFSTC
#include "wtk/core/wtk_type.h" 
#include "wtk/lex/wtk_lex.h"
#include "wtk_semfstr.h"
#include "wtk/core/wtk_os.h"
#include "wtk/core/wtk_str_encode.h"
#include "wtk/core/wtk_str_parser.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_semfstc wtk_semfstc_t;

typedef enum
{
	WTK_SEMFSTC_INIT=0,
	WTK_SEMFSTC_COMMENT,
	WTK_SEMFSTC_INCLUDE,
	WTK_SEMFSTC_ATTR,
	WTK_SEMFSTC_STRING,
	WTK_SEMFSTC_LEX,
	WTK_SEMFSTC_OUTPUT,
}wtk_semfstc_state_t;

typedef enum
{
	WTK_SEMFSTR_INIT,
	WTK_SEMFSTR_SCEEN,
	WTK_SEMFSTR_SCEEN_SCRIPT,
	WTK_SEMFSTR_SCEEN_SCRIPT_LEX,
	WTK_SEMFSTR_SCEEN_SCRIPT_OUTPUT,
}wtk_semfstr_state_t;

struct wtk_semfstc
{
	wtk_heap_t *heap;
	wtk_rbin2_t *rbin;
	wtk_strbuf_t *buf;
	wtk_strbuf_t *tmp;
	wtk_string_t *pwd;
	wtk_semfstc_state_t state;
	int sub_state;
	wtk_string_parser_t str_parser;
	wtk_semfstc_state_t str_bak_state;
	int str_bak_sub_state;
	wtk_semfst_net_t *net;
	wtk_semfst_sceen_t *sceen;
	wtk_semfst_script_t *script;
	wtk_semfst_output_t *output;
	wtk_semfstr_state_t r_state;
	wtk_larray_t *a;
};


wtk_semfstc_t* wtk_semfstc_new();
void wtk_semfstc_delete(wtk_semfstc_t *fst);
void wtk_semfstc_reset(wtk_semfstc_t *c);
int wtk_semfstc_compile_file(wtk_semfstc_t *fst,wtk_semfst_net_t *net,char *fn);
#ifdef __cplusplus
};
#endif
#endif
