#ifndef WTK_SEMDLG_SEMFLD_WTK_SEMSLOT_CFG
#define WTK_SEMDLG_SEMFLD_WTK_SEMSLOT_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_semslot_cfg wtk_semslot_cfg_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_string_t name;
	wtk_string_t ename;
	wtk_string_t ask;
	unsigned can_be_nil:1;
}wtk_semslot_item_cfg_t;

struct wtk_semslot_cfg
{
	wtk_queue_t slot_q;
};

int wtk_semslot_cfg_init(wtk_semslot_cfg_t *cfg);
int wtk_semslot_cfg_clean(wtk_semslot_cfg_t *cfg);
int wtk_semslot_cfg_update_local(wtk_semslot_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_semslot_cfg_update(wtk_semslot_cfg_t *cfg);
wtk_semslot_item_cfg_t* wtk_semslot_cfg_get(wtk_semslot_cfg_t *cfg,char *k,int klen);
#ifdef __cplusplus
};
#endif
#endif
