#include "qtk_usc_cfg.h"


int qtk_usc_cfg_init(qtk_usc_cfg_t *cfg) {
    cfg->app_key = "lbpimyv6v5ljigatepkvdmyeh4regdhvxwzlleq6";
    cfg->secret_key = "b22ca56032384571074178de91f69f45";
    return 0;
}


int qtk_usc_cfg_clean(qtk_usc_cfg_t *cfg) {
    return 0;
}


int qtk_usc_cfg_update_local(qtk_usc_cfg_t *cfg, wtk_local_cfg_t *lc) {
    wtk_string_t *v;

    wtk_local_cfg_update_cfg_str(lc, cfg, app_key, v);
    wtk_local_cfg_update_cfg_str(lc, cfg, secret_key, v);
    return 0;
}


int qtk_usc_cfg_update(qtk_usc_cfg_t *cfg) {
    return 0;
}
