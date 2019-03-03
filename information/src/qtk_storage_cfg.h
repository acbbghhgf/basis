#ifndef _QTK_CACHED_QTK_STORAGE_CFG_H
#define _QTK_CACHED_QTK_STORAGE_CFG_H
#include "wtk/core/cfg/wtk_cfg_file.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct qtk_storage_cfg qtk_storage_cfg_t;

struct qtk_storage_cfg {
    int wtopic_hash_size;
    int data_chunk_size;
    int active_index_expire_time;
    char *data_dir;
};

int qtk_storage_cfg_init(qtk_storage_cfg_t *cfg);
int qtk_storage_cfg_clean(qtk_storage_cfg_t *cfg);
int qtk_storage_cfg_update(qtk_storage_cfg_t *cfg);
int qtk_storage_cfg_update_local(qtk_storage_cfg_t *cfg, wtk_local_cfg_t *lc);
void qtk_storage_cfg_print(qtk_storage_cfg_t *cfg);

#ifdef __cplusplus
}
#endif

#endif
