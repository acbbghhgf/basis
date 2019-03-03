#ifndef WTK_LEX_WRDVEC_WTK_VECDB
#define WTK_LEX_WRDVEC_WTK_VECDB
#include "wtk/core/wtk_type.h" 
#include "wtk/core/segmenter/wtk_vecfn.h"
#include "wtk_vecdb_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_vecdb wtk_vecdb_t;

struct wtk_vecdb {
    wtk_vecdb_cfg_t *cfg;
    wtk_wrdvec_t *wrdvec;
    wtk_vecfn_t *vecfn;
    wtk_strbuf_t *buf;
};

wtk_vecdb_t* wtk_vecdb_new(wtk_vecdb_cfg_t *cfg);
void wtk_vecdb_delete(wtk_vecdb_t *v);
void wtk_vecdb_add(wtk_vecdb_t *v, char *q, int q_bytes, char *a, int a_bytes);
wtk_string_t wtk_vecdb_get(wtk_vecdb_t *v, char *q, int q_bytes);
#ifdef __cplusplus
}
;
#endif
#endif
