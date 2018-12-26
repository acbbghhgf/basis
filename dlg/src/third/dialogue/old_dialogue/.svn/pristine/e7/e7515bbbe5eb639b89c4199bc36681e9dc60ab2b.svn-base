#ifndef WTK_SEMDLG_FAQ_WTK_FAQC
#define WTK_SEMDLG_FAQ_WTK_FAQC
#include "wtk/core/wtk_type.h" 
#include "wtk_faqc_cfg.h"
#include "wtk/os/wtk_thread.h"
#include "wtk/core/json/wtk_json_parse.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_faqc wtk_faqc_t;
#define wtk_faqc_query_start_s(f,s) wtk_faqc_query_start(f,s,sizeof(s)-1)

struct wtk_faqc
{
	wtk_faqc_cfg_t *cfg;
	wtk_httpc_t *httpc;
	wtk_thread_t thread;
	wtk_strbuf_t *buf;
	wtk_sem_t r_sem;
	float cur_conf;
	unsigned run:1;
};


wtk_faqc_t* wtk_faqc_new(wtk_faqc_cfg_t *cfg);
void wtk_faqc_delete(wtk_faqc_t *faq);
void wtk_faqc_query_start(wtk_faqc_t *faq,char *data,int bytes);
wtk_string_t wtk_faqc_get_query(wtk_faqc_t *faq);
wtk_string_t wtk_faqc_get_query2(wtk_faqc_t *faq, float* conf);
#ifdef __cplusplus
};
#endif
#endif
