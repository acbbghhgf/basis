#ifndef WTK_CORE_WTK_FWIN
#define WTK_CORE_WTK_FWIN
#include "wtk/core/wtk_type.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_fwin wtk_fwin_t;
#define wtk_fwin_add(w,f) ((w)->dat[(w)->pos++]=f)

struct wtk_fwin
{
	float *dat;
	int pos;
	int len;
};

wtk_fwin_t* wtk_fwin_new(int len);
void wtk_fwin_delete(wtk_fwin_t *w);
void wtk_fwin_reset(wtk_fwin_t *w);
int wtk_fwin_push(wtk_fwin_t *w,float *p,int n);
void wtk_fwin_pop(wtk_fwin_t *w,int n);
void wtk_fwin_mul(wtk_fwin_t *w,float xmx);
#ifdef __cplusplus
};
#endif
#endif
