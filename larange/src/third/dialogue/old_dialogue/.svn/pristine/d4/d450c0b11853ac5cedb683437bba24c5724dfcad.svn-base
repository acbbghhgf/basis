#ifndef WTK_LMLEX_WTK_LMLEX_H_
#define WTK_LMLEX_WTK_LMLEX_H_
#include "wtk/core/wtk_type.h"
#include "wtk/lex/lmlex/lmrec/wtk_lmrec.h"
#include "wtk_lmlex_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lmlex wtk_lmlex_t;

struct wtk_lmlex {
    wtk_lmlex_cfg_t *cfg;
    wtk_lmrec_t *lmrec;
    wtk_lmres_t *lmres;
    wtk_heap_t *heap;
};

wtk_lmlex_t* wtk_lmlex_new(wtk_lmlex_cfg_t *cfg);
void wtk_lmlex_delete(wtk_lmlex_t *l);
void wtk_lmlex_reset(wtk_lmlex_t *l);
wtk_string_t wtk_lmlex_process(wtk_lmlex_t *l, char *data, int bytes);
void wtk_lmlex_print(wtk_lmlex_t *l);
#ifdef __cplusplus
}
;
#endif
#endif
