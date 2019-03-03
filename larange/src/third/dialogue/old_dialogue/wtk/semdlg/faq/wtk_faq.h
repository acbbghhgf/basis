#ifndef WTK_SEMDLG_FAQ_WTK_FAQ
#define WTK_SEMDLG_FAQ_WTK_FAQ
#include "wtk/core/wtk_type.h"
#include "wtk_faq_cfg.h"
#include "wtk/lex/wrdvec/wtk_vecdb.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_faq wtk_faq_t;
struct wtk_faq
{
	wtk_faq_cfg_t *cfg;
	union{
		wtk_tfidf_t *idf;
		wtk_vecfaq_t *vecfaq;
	}v;
	//wtk_faqdb_t *db;
	wtk_vecdb_t *db;
};

wtk_faq_t* wtk_faq_new(wtk_faq_cfg_t *cfg,wtk_rbin2_t *rbin);
void wtk_faq_delete(wtk_faq_t *faq);
wtk_string_t wtk_faq_get(wtk_faq_t *faq,char *data,int bytes);
wtk_string_t wtk_faq_get2(wtk_faq_t *faq,char *s,int slen,char *input,int input_len);
void wtk_faq_add(wtk_faq_t *faq,char *q,int q_bytes,char *a,int a_bytes);
void wtk_faq_set_wrdvec(wtk_faq_t *faq,wtk_wrdvec_t *wvec);
#ifdef __cplusplus
};
#endif
#endif
