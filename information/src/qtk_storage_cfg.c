#include "qtk_storage_cfg.h"


int qtk_storage_cfg_init(qtk_storage_cfg_t *cfg) {
    cfg->wtopic_hash_size = 1024;
    cfg->active_index_expire_time = 60; /* index in act_dir no longer than 1 min */
    cfg->data_chunk_size = 1024*1024*512; /* data chunk less than 512M */
    cfg->data_dir = "/dev/shm/iris/";
    return 0;
}


int qtk_storage_cfg_clean(qtk_storage_cfg_t *cfg) {
    return 0;
}


int qtk_storage_cfg_update(qtk_storage_cfg_t *cfg) {
    return 0;
}


int qtk_storage_cfg_update_local(qtk_storage_cfg_t *cfg, wtk_local_cfg_t *main) {
    wtk_local_cfg_t *lc = main;
    wtk_string_t *v;

    wtk_local_cfg_update_cfg_i(lc, cfg, wtopic_hash_size, v);
    wtk_local_cfg_update_cfg_i(lc, cfg, data_chunk_size, v);
    wtk_local_cfg_update_cfg_i(lc, cfg, active_index_expire_time, v);
    wtk_local_cfg_update_cfg_str(lc, cfg, data_dir, v);
    if (cfg->data_dir) {
        int len = strlen(cfg->data_dir);
        if (cfg->data_dir[len-1] != '/') {
            const char* data_dir = cfg->data_dir;
            cfg->data_dir = wtk_heap_malloc(main->heap, len+2);
            if (cfg->data_dir) {
                strcpy(cfg->data_dir, data_dir);
                cfg->data_dir[len] = '/';
                cfg->data_dir[len+1] = '\0';
            }
        }
    }

    return 0;
}


void qtk_storage_cfg_print(qtk_storage_cfg_t *cfg) {
}
