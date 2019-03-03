#ifndef _MOD_QTK_FTY_H_
#define _MOD_QTK_FTY_H_

#include "qtk_variant.h"

void qtk_fty_init();
qtk_variant_t *qtk_fty_pop(const char *k, size_t l);
int qtk_fty_push(qtk_variant_t *var);
void qtk_fty_remove(qtk_variant_t *var);
void qtk_fty_clean();
int qtk_fty_exist(const char *k, size_t l);
int qtk_fty_clear(const char *k, size_t l);

#endif
