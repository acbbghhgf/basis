#include "qtk_director_cfg.h"


int qtk_director_cfg_init(qtk_director_cfg_t *cfg) {
    cfg->clean_time = 1000;
    cfg->file_expire_time = 10000;
    qtk_storage_cfg_init(&cfg->storage);
    return 0;
}


int qtk_director_cfg_clean(qtk_director_cfg_t *cfg) {
    qtk_storage_cfg_clean(&cfg->storage);
    return 0;
}


int qtk_director_cfg_update(qtk_director_cfg_t *cfg) {
    qtk_storage_cfg_update(&cfg->storage);
    return 0;
}


int qtk_director_cfg_update_local(qtk_director_cfg_t *cfg, wtk_local_cfg_t *main) {
    wtk_local_cfg_t *lc = main;
    wtk_string_t *v;
    int ret;

    wtk_local_cfg_update_cfg_i(lc, cfg, clean_time, v);
    wtk_local_cfg_update_cfg_i(lc, cfg, file_expire_time, v);

    lc = wtk_local_cfg_find_lc_s(main, "storage");
    if (lc) {
        ret = qtk_storage_cfg_update_local(&cfg->storage, lc);
        if (ret) goto end;
    }
end:
    return 0;
}


void qtk_director_cfg_print(qtk_director_cfg_t *cfg) {
}
