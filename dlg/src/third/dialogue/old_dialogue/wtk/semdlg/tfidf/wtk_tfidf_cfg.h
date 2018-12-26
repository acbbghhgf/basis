#ifndef WTK_TFIDF_WTK_TFIDF_CFG_H_
#define WTK_TFIDF_WTK_TFIDF_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/core/segmenter/wtk_segmenter.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_tfidf_cfg wtk_tfidf_cfg_t;
struct wtk_tfidf_cfg
{
	wtk_segmenter_cfg_t segmenter;
	wtk_str_hash_t *char_map;
	wtk_str_hash_t *stop_map;
	char *sym_fn;
	char *stop_fn;
	char *bin_fn;
	float idf_thresh;
	wtk_string_t def;
	int nbest;
	unsigned skip_ws:1;
	unsigned use_seg:1;
};

int wtk_tfidf_cfg_init(wtk_tfidf_cfg_t *cfg);
int wtk_tfidf_cfg_clean(wtk_tfidf_cfg_t *cfg);
int wtk_tfidf_cfg_update_local(wtk_tfidf_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_tfidf_cfg_update(wtk_tfidf_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
