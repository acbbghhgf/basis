#ifndef WTK_EVAL_ERRNO_WTK_EOS_H_
#define WTK_EVAL_ERRNO_WTK_EOS_H_
#include "wtk_errno.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_eos wtk_eos_t;
//#define wtk_eos_set_err(e,no,...) wtk_errno_set((e)->err,no,__VA_ARGS__)

struct wtk_eos
{
	wtk_errno_t *err;
};

wtk_eos_t* wtk_eos_new();
int wtk_eos_delete(wtk_eos_t *o);
int wtk_eos_reset(wtk_eos_t *o);
#ifdef __cplusplus
};
#endif
#endif
