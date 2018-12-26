#ifndef WTK_SEMDLG_WTK_SEMDEF
#define WTK_SEMDLG_WTK_SEMDEF
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wtk_fkv2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_semdef wtk_semdef_t;

struct wtk_semdef
{
	wtk_string_t *dn;
	wtk_strbuf_t *buf;
};

wtk_semdef_t* wtk_semdef_new(char *dn);
void wtk_semdef_delete(wtk_semdef_t *def);
int wtk_semdef_get_def(wtk_semdef_t *def,char *sec,int sec_len,char *k,int k_len);
#ifdef __cplusplus
};
#endif
#endif
