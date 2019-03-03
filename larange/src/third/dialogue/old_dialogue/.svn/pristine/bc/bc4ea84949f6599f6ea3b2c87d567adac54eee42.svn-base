#ifndef WTK_SEMDLG_SEMFLD_WTK_SEMSLOT
#define WTK_SEMDLG_SEMFLD_WTK_SEMSLOT
#include "wtk/core/wtk_type.h" 
#include "wtk_semslot_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_semslot wtk_semslot_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_semslot_item_cfg_t *cfg;
	wtk_string_t *k;
	wtk_string_t *v;
}wtk_semslot_item_t;


struct wtk_semslot
{
	wtk_semslot_cfg_t *cfg;
	wtk_queue_t slot_q;
};

wtk_semslot_t* wtk_semslot_new(wtk_semslot_cfg_t *cfg);
void wtk_semslot_delete(wtk_semslot_t *s);
void wtk_semslot_reset(wtk_semslot_t *s);
int wtk_semslot_nslot(wtk_semslot_t *s);
void wtk_semslot_set(wtk_semslot_t *s,char *k,int klen,char *v,int vlen);
wtk_semslot_item_t* wtk_semslot_get(wtk_semslot_t *s,char *k,int klen);
wtk_string_t* wtk_semslot_get2(wtk_semslot_t *s,char *k,int klen);
void wtk_semslot_del(wtk_semslot_t *s,char *k,int klen);
void wtk_semslot_print(wtk_semslot_t *s);
wtk_string_t* wtk_semslot_item_get_key(wtk_semslot_item_t *item);
int wtk_semslot_is_full(wtk_semslot_t *s);
#ifdef __cplusplus
};
#endif
#endif
