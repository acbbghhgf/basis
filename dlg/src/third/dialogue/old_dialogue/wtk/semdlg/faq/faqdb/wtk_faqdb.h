#ifndef WTK_SEMDLG_FAQ_FAQDB_WTK_FAQDB
#define WTK_SEMDLG_FAQ_FAQDB_WTK_FAQDB
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_sqlite.h"
#include "wtk_faqdb_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_faqdb wtk_faqdb_t;

struct wtk_faqdb
{
	wtk_faqdb_cfg_t *cfg;
	//wtk_wrdvec_t *wrdvec;
	wtk_sqlite_t *sqlite;
	wtk_strbuf_t *buf;
	wtk_strbuf_t *buf2;
};

wtk_faqdb_t* wtk_faqdb_new(wtk_faqdb_cfg_t *cfg);
void wtk_faqdb_delete(wtk_faqdb_t *db);
void wtk_faqdb_add(wtk_faqdb_t *db,char *q,int q_bytes,char *a,int a_bytes);
wtk_string_t wtk_faqdb_get(wtk_faqdb_t *db,char *q,int q_bytes);
#ifdef __cplusplus
};
#endif
#endif
