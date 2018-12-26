#include "qtk_zeus_cfg.h"


int qtk_zeus_cfg_init(qtk_zeus_cfg_t *cfg) {
    cfg->log_fn = NULL;
    cfg->hw_auth_host = "cloud.qdreamer.com";
    cfg->hw_auth_port = 80;
    cfg->hw_auth_url = "/authApp/api/v1/serverDeviceVerify/";
    return 0;
}


int qtk_zeus_cfg_clean(qtk_zeus_cfg_t *cfg) {
    return 0;
}


int qtk_zeus_cfg_update(qtk_zeus_cfg_t *cfg) {
    return 0;
}


int qtk_zeus_cfg_update_local(qtk_zeus_cfg_t *cfg, wtk_local_cfg_t *main) {
    wtk_local_cfg_t *lc = main;
    int ret = 0;
    wtk_string_t *v;

    wtk_local_cfg_update_cfg_str(lc, cfg, log_fn, v);
    wtk_local_cfg_update_cfg_str(lc, cfg, hw_auth_host, v);
    wtk_local_cfg_update_cfg_str(lc, cfg, hw_auth_url, v);
    wtk_local_cfg_update_cfg_i(lc, cfg, hw_auth_port, v);

    return ret;
}
