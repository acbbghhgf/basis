#ifndef WTK_RNN_WTK_WRDVEC_CFG
#define WTK_RNN_WTK_WRDVEC_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/core/math/wtk_mat.h"
#include "wtk/core/segmenter/wtk_segmenter.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_wrdvec_cfg wtk_wrdvec_cfg_t;

typedef struct {
    wtk_string_t *name;
    wtk_vecf_t *m;
    int wrd_idx;
} wtk_wrdvec_item_t;

typedef struct {
    wtk_wrdvec_item_t *item;
    float f;
} wtk_wrdvec_cmp_t;

struct wtk_wrdvec_cfg {
    wtk_segmenter_cfg_t seg;
    int voc_size;
    int vec_size;
    char *fn;
    wtk_str_hash_t *hash;
    wtk_wrdvec_item_t **wrds;
    unsigned int offset;
    unsigned int use_bin :1;
};

int wtk_wrdvec_cfg_init(wtk_wrdvec_cfg_t *cfg);
int wtk_wrdvec_cfg_clean(wtk_wrdvec_cfg_t *cfg);
int wtk_wrdvec_cfg_update_local(wtk_wrdvec_cfg_t *cfg, wtk_local_cfg_t *lc);
int wtk_wrdvec_cfg_update(wtk_wrdvec_cfg_t *cfg);
int wtk_wrdvec_cfg_update2(wtk_wrdvec_cfg_t *cfg, wtk_source_loader_t *sl);
wtk_wrdvec_item_t* wtk_wrdvec_cfg_find(wtk_wrdvec_cfg_t *cfg, char *data,
        int bytes);
void wtk_wrdvec_cfg_test(wtk_wrdvec_cfg_t *cfg, char *s1, int s1_bytes,
        char *s2, int s2_bytes);
void wtk_wrdvec_cfg_test2(wtk_wrdvec_cfg_t *cfg, char *s1, int s1_bytes);
void wtk_wrdvec_cfg_test3(wtk_wrdvec_cfg_t *cfg, wtk_vecf_t *v1);
void wtk_wrdvec_cfg_write_bin(wtk_wrdvec_cfg_t *cfg, char *fn);
#ifdef __cplusplus
}
;
#endif
#endif
