#ifndef WTK_LEX_WTK_LEX
#define WTK_LEX_WTK_LEX
#include "wtk/core/wtk_type.h" 
#include "wtk_lex_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lex wtk_lex_t;

struct wtk_lex {
    wtk_lex_cfg_t *cfg;
    wtk_lexc_t *lexc;
    wtk_lexr_t *lexr;
    wtk_lex_script_t *script;
    wtk_lex_net_t *net;
};

wtk_lex_t* wtk_lex_new(wtk_lex_cfg_t *cfg);
void wtk_lex_delete(wtk_lex_t *l);
void wtk_lex_reset(wtk_lex_t *l);
int wtk_lex_compile(wtk_lex_t *l, char *fn);
int wtk_lex_compile2(wtk_lex_t *l, char *data, int bytes);
int wtk_lex_compile3(wtk_lex_t *l, wtk_source_t *src);
wtk_string_t wtk_lex_process(wtk_lex_t *l, char *data, int bytes);
wtk_string_t wtk_lex_match(wtk_lex_t *l, char *lex, int lex_bytes, char *data,
        int data_bytes);
#ifdef __cplusplus
}
;
#endif
#endif
