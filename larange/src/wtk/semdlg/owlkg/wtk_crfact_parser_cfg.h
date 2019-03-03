#ifndef WTK_SEMDLG_OWLKG_WTK_CRFACT_PARSER_CFG
#define WTK_SEMDLG_OWLKG_WTK_CRFACT_PARSER_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/core/segmenter/wtk_poseg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_crfact_parser_cfg wtk_crfact_parser_cfg_t;
struct wtk_crfact_parser_cfg
{
	wtk_poseg_cfg_t poseg;
	char *model;
};

int wtk_crfact_parser_cfg_init(wtk_crfact_parser_cfg_t *cfg);
int wtk_crfact_parser_cfg_clean(wtk_crfact_parser_cfg_t *cfg);
int wtk_crfact_parser_cfg_update_local(wtk_crfact_parser_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_crfact_parser_cfg_update(wtk_crfact_parser_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
