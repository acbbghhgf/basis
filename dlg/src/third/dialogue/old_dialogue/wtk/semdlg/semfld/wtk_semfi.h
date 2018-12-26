#ifndef WTK_SEMDLG_SEMFLD_WTK_SEMFI
#define WTK_SEMDLG_SEMFLD_WTK_SEMFI
#include "wtk/core/wtk_type.h" 
#include "wtk/semdlg/owlkg/wtk_crf.h"
#include "wtk_act_lua.h"
#include "wtk_semfi_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_semfi wtk_semfi_t;
struct wtk_semfld;

struct wtk_semfi
{
	wtk_semfi_cfg_t *cfg;
#ifdef USE_CRF
	wtk_crf_t *crf;
#endif
	wtk_str_hash_t *crf_hash;
	struct wtk_semfld *fld;
	//wtk_json_item_t *ji;
	wtk_act_t act;
};

wtk_semfi_t* wtk_semfi_new(wtk_semfi_cfg_t *cfg,struct wtk_semfld *fld);
void wtk_semfi_delete(wtk_semfi_t *semfi);
void wtk_semfi_reset(wtk_semfi_t *semfi);
int wtk_semfi_process(wtk_semfi_t *semfi,char *data,int bytes);
#ifdef __cplusplus
};
#endif
#endif
