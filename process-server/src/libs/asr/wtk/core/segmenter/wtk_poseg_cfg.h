#ifndef WTK_CORE_SEGMENTER_WTK_POSEG_CFG
#define WTK_CORE_SEGMENTER_WTK_POSEG_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk_chnpos_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_poseg_cfg wtk_poseg_cfg_t;

//python py/wtk/kv2bin2.py -i ./res/pos/dict.txt.big   -o test.bin -type "float|string"
struct wtk_poseg_cfg
{
	wtk_chnpos_cfg_t pos;
	wtk_array_t *filter;
	wtk_posdict_t *dict;
	int def_weight;
	int max_char;
	char *dict_fn;
	unsigned upper:1;
	unsigned use_dict_bin:1;
};

int wtk_poseg_cfg_init(wtk_poseg_cfg_t *cfg);
int wtk_poseg_cfg_clean(wtk_poseg_cfg_t *cfg);
int wtk_poseg_cfg_update_local(wtk_poseg_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_poseg_cfg_update(wtk_poseg_cfg_t *cfg);
int wtk_poseg_cfg_update2(wtk_poseg_cfg_t *cfg,wtk_source_loader_t *sl);
#ifdef __cplusplus
};
#endif
#endif
