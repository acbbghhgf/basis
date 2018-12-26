#ifndef WTK_FST_STRLIKE_WTK_STRLIKE_CFG_H_
#define WTK_FST_STRLIKE_WTK_STRLIKE_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_strlike_cfg wtk_strlike_cfg_t;

struct wtk_strlike_cfg
{
	wtk_str_hash_t *en_filter;
	wtk_str_hash_t *cn_filter;
	wtk_str_hash_t *sp_filter;//空格
	float cost_del;
	float cost_ins;
	float cost_sub;
	unsigned int use_eq_len:1;
	unsigned int use_char:1;
};

int wtk_strlike_cfg_init(wtk_strlike_cfg_t *cfg);
int wtk_strlike_cfg_clean(wtk_strlike_cfg_t *cfg);
int wtk_strlike_cfg_update_local(wtk_strlike_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_strlike_cfg_update(wtk_strlike_cfg_t *cfg);

int wtk_strlike_cfg_is_en(wtk_strlike_cfg_t *cfg,char *data,int bytes);
int wtk_strlike_cfg_is_cn(wtk_strlike_cfg_t *cfg,char *data,int bytes);
int wtk_strlike_cfg_is_sp(wtk_strlike_cfg_t *cfg,char *data,int bytes);
#ifdef __cplusplus
};
#endif
#endif
