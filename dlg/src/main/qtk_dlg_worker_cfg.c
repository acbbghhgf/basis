#include "qtk_dlg_worker_cfg.h"
#include "qtk/os/qtk_alloc.h"
#include "qtk/os/qtk_base.h"

int qtk_dlg_worker_cfg_init(qtk_dlg_worker_cfg_t *cfg)
{
    qtk_event_cfg_init(&cfg->ev_cfg);
    wtk_queue_init(&cfg->inst_cfgs);
    cfg->lua = NULL;
    cfg->lua_path = NULL;
    cfg->lua_cpath = NULL;
    cfg->tmp = NULL;
    cfg->mod_path = NULL;
    cfg->max_raw_cnt = 0;
    cfg->max_pending = 0;
    return 0;
}

int qtk_dlg_worker_cfg_clean(qtk_dlg_worker_cfg_t *cfg)
{
    qtk_event_cfg_clean(&cfg->ev_cfg);
    wtk_queue_node_t *node;
    while ((node = wtk_queue_pop(&cfg->inst_cfgs))) {
        qtk_inst_cfg_t *inst_cfg = data_offset(node, qtk_inst_cfg_t, node);
        qtk_inst_cfg_clean(inst_cfg);
        qtk_free(inst_cfg);
    }
    return 0;
}

int qtk_dlg_worker_cfg_update(qtk_dlg_worker_cfg_t *cfg)
{
    qtk_event_cfg_update(&cfg->ev_cfg);
    return 0;
}

static int _worker_update_mod_cfg(qtk_dlg_worker_cfg_t *cfg, wtk_local_cfg_t *lc)
{
    wtk_string_t *v;
    wtk_local_cfg_update_cfg_i(lc, cfg, max_raw_cnt, v);
    wtk_local_cfg_update_cfg_i(lc, cfg, max_pending, v);
    wtk_array_t *a = wtk_local_cfg_find_array_s(lc, "inst");
    if (!a) return -1;
    wtk_string_t **elems = a->slot;
    uint32_t i;
    for (i=0; i<a->nslot; i++) {
        wtk_local_cfg_t *ic = wtk_local_cfg_find_lc(lc, elems[i]->data, elems[i]->len);
        if (!ic) return -1;
        qtk_inst_cfg_t *inst_cfg = qtk_calloc(1, sizeof(*inst_cfg));
        if (qtk_inst_cfg_init(inst_cfg)) return -1;
        if (qtk_inst_cfg_update_local(inst_cfg, ic)) return -1;
        if (qtk_inst_cfg_update(inst_cfg)) return -1;
        wtk_queue_push(&cfg->inst_cfgs, &inst_cfg->node);
        if (inst_cfg->tmp) cfg->tmp = inst_cfg;
    }

    return 0;
}

int qtk_dlg_worker_cfg_update_local(qtk_dlg_worker_cfg_t *cfg, wtk_local_cfg_t *main)
{
    wtk_local_cfg_t *lc = NULL;
    wtk_string_t *v;
    wtk_local_cfg_update_cfg_str(main, cfg, lua, v);
    wtk_local_cfg_update_cfg_str(main, cfg, mod_path, v);
    wtk_local_cfg_update_cfg_string(main, cfg, lua_path, v);
    wtk_local_cfg_update_cfg_string(main, cfg, lua_cpath, v);
    lc = wtk_local_cfg_find_lc_s(main, "event");
    if (lc && qtk_event_cfg_update_local(&cfg->ev_cfg, lc))
        return -1;

    lc = wtk_local_cfg_find_lc_s(main, "mod");
    if (lc && _worker_update_mod_cfg(cfg, lc))
        return -1;

    return 0;
}

qtk_inst_cfg_t *qtk_dlg_worker_find_inst_cfg(qtk_dlg_worker_cfg_t *cfg,
                                             const char *res, size_t len)
{
    wtk_queue_node_t *node = cfg->inst_cfgs.pop;
    for (;node;node=node->next) {
        qtk_inst_cfg_t *inst_cfg = data_offset(node, qtk_inst_cfg_t, node);
        /*if (0 == memcmp(inst_cfg->cfn, (char *)res, len) && inst_cfg->cfn[len] == '\0') {*/
        if (0 == wtk_string_cmp(inst_cfg->res, cast(char *, res), len)) {
            return inst_cfg;
        }
    }
    return cfg->tmp;
}
