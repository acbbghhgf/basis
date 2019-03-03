#ifdef USE_STATICS
#ifndef WTK_HTTP_LOC_STATICS_WTK_STATICS_H_
#define WTK_HTTP_LOC_STATICS_WTK_STATICS_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_stack.h"
#include "wtk/speech/wtk_speech.h"
#include "wtk/vm/wtk_vm.h"
#include "wtk_statics_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif

void wtk_loc_statics_set_vars(wtk_speech_t *speech,wtk_vm_t *vm);
void wtk_loc_attach_statics2(wtk_stack_t *stk,wtk_nk_t *nk);
void wtk_loc_attach_statics(wtk_stack_t *stk,wtk_nk_t *nk);
int wtk_loc_statics_process(void *data,wtk_request_t *req);
int wtk_loc_debug_process(void *data,wtk_request_t *req);
int wtk_loc_speech_process(void *data,wtk_request_t *req);
int wtk_loc_cpu_process(void *data,wtk_request_t *req);
#ifdef __cplusplus
};
#endif
#endif
#endif
