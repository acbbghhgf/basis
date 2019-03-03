#ifndef WTK_SEMDLG_FAQ_WTK_FAQ_CFG
#define WTK_SEMDLG_FAQ_WTK_FAQ_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/semdlg/tfidf/wtk_tfidf.h"
#include "wtk/lex/wrdvec/wtk_vecfaq.h"
#include "wtk/semdlg/faq/faqdb/wtk_faqdb.h"
#include "wtk/lex/wrdvec/wtk_vecdb.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_faq_cfg wtk_faq_cfg_t;

struct wtk_faq_cfg
{
	wtk_tfidf_cfg_t tifidf;
	wtk_vecfaq_cfg_t vecfaq;
	wtk_vecdb_cfg_t vecdb;
	unsigned use_tfidf:1;
	unsigned use_vecdb:1;
};

int wtk_faq_cfg_init(wtk_faq_cfg_t *cfg);
int wtk_faq_cfg_clean(wtk_faq_cfg_t *cfg);
int wtk_faq_cfg_update_local(wtk_faq_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_faq_cfg_update(wtk_faq_cfg_t *cfg);
int wtk_faq_cfg_update2(wtk_faq_cfg_t *cfg,wtk_source_loader_t *sl);
#ifdef __cplusplus
};
#endif
#endif
