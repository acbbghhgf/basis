#ifndef _QTK_CACHED_QTK_DIRECTOR_CFG_H
#define _QTK_CACHED_QTK_DIRECTOR_CFG_H
#include "wtk/core/cfg/wtk_cfg_file.h"
#include "qtk_storage_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct qtk_director_cfg qtk_director_cfg_t;

struct qtk_director_cfg {
    qtk_storage_cfg_t storage;
    int clean_time;
    int file_expire_time;
};

int qtk_director_cfg_init(qtk_director_cfg_t *cfg);
int qtk_director_cfg_clean(qtk_director_cfg_t *cfg);
int qtk_director_cfg_update(qtk_director_cfg_t *cfg);
int qtk_director_cfg_update_local(qtk_director_cfg_t *cfg, wtk_local_cfg_t *lc);
void qtk_director_cfg_print(qtk_director_cfg_t *cfg);

#ifdef __cplusplus
}
#endif

#endif
