#ifndef _MOD_QTK_INST_H_
#define _MOD_QTK_INST_H_

#include "qtk_variant.h"
#include "qtk_inst_cfg.h"

qtk_variant_t *qtk_inst_new(qtk_inst_cfg_t *cfg, const char *path, size_t len);
int qtk_inst_mod_init(const char *fn);
void qtk_inst_mod_clean();

#endif
