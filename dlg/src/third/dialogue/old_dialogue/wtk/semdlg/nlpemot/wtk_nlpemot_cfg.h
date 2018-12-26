#ifndef WTK_SEMDLG_NLPEMOT_WTK_NLPEMOT_CFG
#define WTK_SEMDLG_NLPEMOT_WTK_NLPEMOT_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/core/segmenter/wtk_segmenter.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/cfg/wtk_source.h"
#include "wtk/lex/wtk_lex.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nlpemot_cfg wtk_nlpemot_cfg_t;

struct wtk_nlpemot_cfg
{
	wtk_segmenter_cfg_t segmenter;
	wtk_str_hash_t *not_hash;
	wtk_str_hash_t *verry_hash;
	wtk_array_t *sep;
	char *dict_fn;
	char *not_fn;
	char *verry_fn;
	char *lex_fn;
	wtk_lex_net_t *lex_net;
	float min;
	float max;
};

int wtk_nlpemot_cfg_init(wtk_nlpemot_cfg_t *cfg);
int wtk_nlpemot_cfg_clean(wtk_nlpemot_cfg_t *cfg);
int wtk_nlpemot_cfg_update_local(wtk_nlpemot_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_nlpemot_cfg_update(wtk_nlpemot_cfg_t *cfg,wtk_lexc_t *lex);
int wtk_nlpemot_cfg_update2(wtk_nlpemot_cfg_t *cfg,wtk_source_loader_t *sl,wtk_lexc_t *lex);
#ifdef __cplusplus
};
#endif
#endif
