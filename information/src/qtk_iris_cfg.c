#include "qtk_iris_cfg.h"
#include "qtk_director_cfg.h"


int qtk_iris_cfg_init(qtk_iris_cfg_t *cfg) {
    cfg->log_fn = NULL;
    cfg->listen = "0.0.0.0";
    cfg->port = 0;
    cfg->backlog = 5;
    qtk_director_cfg_init(&cfg->director);
    return 0;
}


int qtk_iris_cfg_clean(qtk_iris_cfg_t *cfg) {
    qtk_director_cfg_clean(&cfg->director);
    return 0;
}


int qtk_iris_cfg_update(qtk_iris_cfg_t *cfg) {
    qtk_director_cfg_update(&cfg->director);
    return 0;
}


int qtk_iris_cfg_update_local(qtk_iris_cfg_t *cfg, wtk_local_cfg_t *main) {
    wtk_local_cfg_t *lc = main;
    int ret = 0;
    wtk_string_t *v;

    wtk_local_cfg_update_cfg_str(lc, cfg, log_fn, v);
    wtk_local_cfg_update_cfg_str(lc, cfg, listen, v);
    wtk_local_cfg_update_cfg_i(lc, cfg, port, v);
    wtk_local_cfg_update_cfg_i(lc, cfg, backlog, v);

    lc = wtk_local_cfg_find_lc_s(main, "director");
    if (lc) {
        ret = qtk_director_cfg_update_local(&cfg->director, lc);
        if (ret) goto end;
    }
end:
    return ret;
}
