#ifndef _MOD_QTK_INCUB_H_
#define _MOD_QTK_INCUB_H_

#include "qtk_variant.h"

void qtk_incub_init();
int qtk_incub_add(qtk_variant_t *var);
qtk_variant_t *qtk_incub_remove(const char *k, size_t l);
qtk_variant_t *qtk_incub_find(const char *k, size_t l);
void qtk_incub_clean();
int qtk_incub_exist(const char *k, size_t l);
int qtk_incub_clear(const char *k, size_t l);

#endif
