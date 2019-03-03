#ifndef WTK_LEX_LEXR_WTK_TREEBIN
#define WTK_LEX_LEXR_WTK_TREEBIN
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_str_encode.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/rbin/wtk_rbin2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_treebin wtk_treebin_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_string_t *nm;
    unsigned int offset;
    unsigned int step;
    unsigned int nslot;
    unsigned int *slot_idx;
} wtk_treebin_tbl_t;

typedef struct {
    wtk_treebin_tbl_t *tbl;
    unsigned int offset;
    unsigned short idx;
    unsigned short is_end :1;
    unsigned short is_err :1;
} wtk_treebin_env_t;

struct wtk_treebin {
    FILE *f;
    wtk_str_hash_t *hash;
    wtk_str_hash_t *section;
    unsigned int max;
    unsigned int nsection;
    unsigned int eof_offset;
    wtk_queue_t section_q;
};

wtk_treebin_t* wtk_treebin_new(char *fn, wtk_rbin2_t *rbin);
void wtk_treebin_delete(wtk_treebin_t *t);

void wtk_treebin_env_init(wtk_treebin_env_t *e);
void wtk_treebin_env_print(wtk_treebin_env_t *e);
int wtk_treebin_has2(wtk_treebin_t *t, wtk_treebin_env_t *env, char *data,
        int bytes);
wtk_treebin_env_t wtk_treebin_has(wtk_treebin_t *t, char *nm, int nm_bytes,
        char *data, int bytes);
wtk_treebin_tbl_t* wtk_treebin_find_tbl(wtk_treebin_t *t, char *nm,
        int nm_bytes);
int wtk_treebin_search2(wtk_treebin_t *t, wtk_treebin_env_t *e, char *data,
        int bytes);
#ifdef __cplusplus
}
;
#endif
#endif
