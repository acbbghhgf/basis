#ifndef WTK_CORE_PARAM_SERIALIZE_H_
#define WTK_CORE_PARAM_SERIALIZE_H_
#include "wtk_param.h"
#include "wtk_module_param.h"
#ifdef __cplusplus
extern "C" {
#endif
struct wtk_stack;
struct wtk_heap;
int wtk_stack_write_params(struct wtk_stack* s,wtk_param_t** params,int count);
wtk_param_t* wtk_stack_read_param(struct wtk_stack* chars);//,struct wtk_heap *heap);
int wtk_param_write(wtk_param_t* param,struct wtk_stack* chars);
void wtk_module_param_write(wtk_module_param_t *param,struct wtk_stack *s);
int wtk_module_param_read(wtk_module_param_t *param,struct wtk_stack *s);
#ifdef __cplusplus
};
#endif
#endif
