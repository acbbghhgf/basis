#include "wtk_lexr_lib.h" 

wtk_lexr_lib_t* wtk_lexr_lib_new(wtk_lexr_lib_cfg_t *cfg, wtk_rbin2_t *rbin)
{
    wtk_lexr_lib_t *lib;

    lib = (wtk_lexr_lib_t*) wtk_malloc(sizeof(wtk_lexr_lib_t));
    lib->cfg = cfg;
    if (cfg->tree_fn) {
        lib->tree = wtk_treebin_new(cfg->tree_fn, rbin);
    } else {
        lib->tree = NULL;
    }
    return lib;
}

void wtk_lexr_lib_delete(wtk_lexr_lib_t *l)
{
    if (l->tree) {
        wtk_treebin_delete(l->tree);
    }
    wtk_free(l);
}

wtk_treebin_env_t wtk_lexr_lib_get_env(wtk_lexr_lib_t *l, char *var,
        int var_bytes)
{
    wtk_treebin_env_t env;
    wtk_lexr_lib_item_t *item;

    wtk_treebin_env_init(&(env));
    if (!l->tree) {
        goto end;
    }
    if (l->cfg->map_hash) {
        item = (wtk_lexr_lib_item_t*) wtk_str_hash_find(l->cfg->map_hash, var,
                var_bytes);
        if (item && item->value) {
            var = item->value->data;
            var_bytes = item->value->len;
        }
    }
    env.tbl = wtk_treebin_find_tbl(l->tree, var, var_bytes);
    end: return env;
}

int wtk_lexr_lib_search(wtk_lexr_lib_t *l, wtk_treebin_env_t *env, char *v,
        int v_bytes)
{
    return wtk_treebin_search2(l->tree, env, v, v_bytes);
}
