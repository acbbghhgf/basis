#ifndef WTK_CORE_WTK_FRING
#define WTK_CORE_WTK_FRING
#include "wtk/core/wtk_type.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_fring wtk_fring_t;
#define wtk_fring_at(r,i) ((r)->f[((r)->first+(i))%((r)->nslot)])

struct wtk_fring
{
	float *f;
	int first;
	int used;
	int nslot;
	float tot_v;
};

wtk_fring_t* wtk_fring_new(int n);
void wtk_fring_delete(wtk_fring_t *r);
void wtk_fring_reset(wtk_fring_t *r);
void wtk_fring_push(wtk_fring_t *r,float f);
float wtk_fring_pop(wtk_fring_t* r);
void wtk_fring_pop2(wtk_fring_t *r,int n);
void wtk_fring_pop3(wtk_fring_t *r,int n);
void wtk_fring_print(wtk_fring_t *r);
#ifdef __cplusplus
};
#endif
#endif
