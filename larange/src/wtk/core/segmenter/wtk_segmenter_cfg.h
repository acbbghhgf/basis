#ifndef WTK_SEGMENTER_WTK_SEGMENTER_CFG_H_
#define WTK_SEGMENTER_WTK_SEGMENTER_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/wtk_fkv2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_segmenter_cfg wtk_segmenter_cfg_t;

/*
 *semdlg vocab to bin:py/wtk/kv2bin2.py
 */

typedef struct
{
	wtk_string_t *wrd;
	double prob;
}wtk_segwrd_t;

struct wtk_segmenter_cfg
{
	wtk_string_t pre;
	wtk_string_t suf;
	wtk_array_t *filter;
	wtk_str_hash_t *hash;
	//wtk_fkv2_t *fkv;
	char *dict_fn;
	int nslot;
	int def_weight;
	int max_char;
	int max_line;
	unsigned upper:1;
	unsigned lower:1;
	unsigned use_bin:1;
};

int wtk_segmenter_cfg_init(wtk_segmenter_cfg_t *cfg);
int wtk_segmenter_cfg_clean(wtk_segmenter_cfg_t *cfg);
int wtk_segmenter_cfg_update_local(wtk_segmenter_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_segmenter_cfg_update(wtk_segmenter_cfg_t *cfg);
int wtk_segmenter_cfg_update2(wtk_segmenter_cfg_t *cfg,wtk_source_loader_t *sl);
#ifdef __cplusplus
};
#endif
#endif
