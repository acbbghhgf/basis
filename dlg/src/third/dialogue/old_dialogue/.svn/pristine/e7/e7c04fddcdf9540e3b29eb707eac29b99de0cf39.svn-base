#include "wtk_lex.h"

wtk_lex_t* wtk_lex_new(wtk_lex_cfg_t *cfg)
{
    wtk_lex_t *l;

    l = (wtk_lex_t*) wtk_malloc(sizeof(wtk_lex_t));
    l->cfg = cfg;
    l->lexc = wtk_lexc_new(&(cfg->lexc));
    l->lexr = wtk_lexr_new(&(cfg->lexr), NULL);
    l->script = NULL;
    l->net = NULL;

    return l;
}

void wtk_lex_delete(wtk_lex_t *l)
{
    if (l->net) {
        wtk_lex_net_delete(l->net);
    }
    if (l->script) {
        wtk_lex_script_delete(l->script);
    }
    wtk_lexc_delete(l->lexc);
    wtk_lexr_delete(l->lexr);
    wtk_free(l);
}

void wtk_lex_reset(wtk_lex_t *l)
{
    wtk_lexr_reset(l->lexr);
}

int wtk_lex_compile(wtk_lex_t *l, char *fn)
{
    int ret = -1;

    l->script = wtk_lexc_compile_file(l->lexc, fn);
    if (!l->script) {
        goto end;
    }
    //wtk_lex_script_print(l->script);
    //exit(0);
    l->net = wtk_lex_net_new(l->script);
    if (!l->net) {
        goto end;
    }
    ret = 0;
    end: return ret;
}

int wtk_lex_compile2(wtk_lex_t *l, char *data, int bytes)
{
    int ret = -1;

    l->script = wtk_lexc_compile(l->lexc, data, bytes);
    if (!l->script) {
        goto end;
    }
    l->net = wtk_lex_net_new(l->script);
    if (!l->net) {
        goto end;
    }
    ret = 0;
    end: return ret;
}

int wtk_lex_compile3(wtk_lex_t *l, wtk_source_t *src)
{
    int ret = -1;

    l->script = wtk_lexc_compile_source(l->lexc, src);
    if (!l->script) {
        goto end;
    }
    l->net = wtk_lex_net_new(l->script);
    if (!l->net) {
        goto end;
    }
    ret = 0;
    end: return ret;
}

wtk_string_t wtk_lex_process(wtk_lex_t *l, char *data, int bytes)
{
    wtk_string_t v;
    int ret;

    wtk_string_set(&(v), 0, 0);
    ret = wtk_lexr_process(l->lexr, l->net, data, bytes);
    if (ret != 0) {
        goto end;
    }
    v = wtk_lexr_get_result(l->lexr);
    end: return v;
}

wtk_string_t wtk_lex_match(wtk_lex_t *l, char *lex, int lex_bytes, char *data,
        int data_bytes)
{
    wtk_lex_net_t *net = NULL;
    int ret = -1;
    wtk_string_t v;

    wtk_string_set(&(v), 0, 0);
    net = wtk_lexc_compile_str(l->lexc, lex, lex_bytes);
    if (!net) {
        goto end;
    }
    wtk_lexr_reset(l->lexr);
    ret = wtk_lexr_process(l->lexr, net, data, data_bytes);
    if (ret != 0) {
        goto end;
    }
    v = wtk_lexr_get_result(l->lexr);
    ret = 0;
    end: if (net) {
        wtk_lex_net_delete(net);
    }
    return v;
}
