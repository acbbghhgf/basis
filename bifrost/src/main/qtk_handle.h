#ifndef MAIN_QTK_CONTEXT_HANDLE_H_
#define MAIN_QTK_CONTEXT_HANDLE_H_

#include <stdint.h>
#include "os/qtk_rwlock.h"

struct qtk_context;

uint32_t qtk_handle_register(struct qtk_context *ctx);
int qtk_handle_retire(uint32_t handle);
struct qtk_context * qtk_handle_grab(uint32_t handle);
void qtk_handle_retireall();

uint32_t qtk_handle_findname(const char * name);
const char * qtk_handle_namehandle(uint32_t handle, const char *name);
void qtk_handle_init();
void qtk_handle_clean();

#endif
