#include "third/lua5.3.4/src/lua.h"
#include "third/lua5.3.4/src/lualib.h"
#include "third/lua5.3.4/src/lauxlib.h"
#include "third/uuid/uuid4.h"
#include "main/qtk_dlg_worker.h"
#include "mod/qtk_variant.h"
#include "mod/qtk_fty.h"
#include "mod/qtk_incub.h"
#include "mod/qtk_inst.h"
#include "mod/qtk_cmd.h"
#include "lua/qtk_lco.h"
#include "mod/qtk_inst_ctl.h"
#include "qtk/os/qtk_base.h"
#include "qtk/os/qtk_alloc.h"

#define LUA_DLGHANDLE   "qtk_variant_t *"

#define tolinst(L, idx)     ((qtk_variant_t *)luaL_checkudata(L, idx, LUA_DLGHANDLE))

static qtk_dlg_worker_t *DW;

static int _getRes_continuation(lua_State *L, int status, lua_KContext ctx)
{
    if (lua_type(L, -1) == LUA_TNUMBER) { //for now, it means getResult timeout
        qtk_debug("TIMEOUT\n");
        return 0;
    }
    luaL_checkstring(L, -1);
    return 1;
}

static int lgetResult(lua_State *L)
{
    qtk_variant_t *var = tolinst(L, 1);
    if (var->closed) return 0;
    int timeout = 500;
    if (lua_isinteger(L, 2)) timeout = lua_tointeger(L, 2);
    int ref = qtk_lreg_yield_co(L, timeout);
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "%d", ref);
    if (qtk_variant_write_cmd(var, GETRES, tmp)) {
        qtk_variant_kill(var, SIGKILL);
        qtk_lunreg_yield_co(L, ref);
        return 0;
    }
    return lua_yieldk(L, 0, 0, _getRes_continuation);
}

static int lclose(lua_State *L)
{
    qtk_variant_t *var = tolinst(L, 1);
    if (var->closed) {
        qtk_free(var);
        return 0;
    }
    var->used = 0;
    if (!var->raw) {
        qtk_fty_push(var);
        qtk_ictl_flush(var->name.data, var->name.len);
        return 0;
    }
    if (qtk_variant_write_cmd(var, ITOZ, NULL)) {
        qtk_variant_kill(var, SIGKILL);
        return 0;
    }
    if (qtk_incub_add(var)) {
        qtk_variant_exit(var);
    }
    return 0;
}

static int lexit(lua_State *L)
{
    qtk_variant_t *var = tolinst(L, 1);
    if (var->closed) {
        qtk_free(var);
        return 0;
    }
    var->used = 0;
    qtk_variant_exit(var);
    return 0;
}

static int lcalc(lua_State *L)
{
    qtk_variant_t *var = tolinst(L, 1);
    if (var->closed) return 0;
    const char *param = luaL_checkstring(L, 2);
    if (qtk_variant_write_cmd(var, CALC, param)) {
        qtk_variant_kill(var, SIGKILL);
        return 0;
    }
    lua_pushboolean(L, 1);
    return 1;
}

static const luaL_Reg libs[] = {
    {"getResult", lgetResult},
    {"close", lclose},
    {"exit", lexit},
    {"calc", lcalc},
    {NULL, NULL}
};

static void createmeta(lua_State *L)
{
    luaL_newmetatable(L, LUA_DLGHANDLE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, libs, 0);
    lua_pop(L, 1);
}

static qtk_variant_t *_new_inst(const char *res, size_t len)
{
    qtk_inst_cfg_t *cfg = qtk_dlg_worker_find_inst_cfg(DW->cfg, res, len);
    if (!cfg) return NULL;
    if (cfg->tmp) {
        if (cfg->cfn) qtk_free(cast(void *, cfg->cfn));
        cfg->cfn = qtk_calloc(1, len + 1);
        memcpy(cfg->cfn, res, len);
    }
    char tmp[UUID4_LEN];
    uuid4_generate(tmp);
    char p[108];
    int l = snprintf(p, sizeof(p), "/tmp/dlgd/%s", tmp);

    qtk_variant_t *v = qtk_inst_new(cfg, p, l);
    if (v) {
        if (qtk_variant_prepare(v, DW->em, DW->ps, p, l)) {
            qtk_variant_kill(v, SIGKILL);
            return NULL;
        }
        v->raw = 1;
        qtk_ictl_update_cnt_s("raw", 1);
    }
    return v;
}

static int _new_continuation(lua_State *L, int status, lua_KContext ctx)
{
    if (lua_type(L, -1) == LUA_TNUMBER) { //for now, it means getResult timeout
        qtk_debug("TIMEOUT\n");
        return 0;
    }

    size_t l;
    const char *arg = luaL_checklstring(L, -1, &l);
    char tmp[l + 1];
    memcpy(tmp, arg, l);
    tmp[l] = '\0';

    char *token = tmp;
    char *path = strsep(&token, ":");
    if (!path) return 0;
    char *pid_s = strsep(&token, ":");
    if (!pid_s) return 0;
    char *name = strsep(&token, ":");
    if (!name) return 0;

    pid_t pid = atoi(pid_s);
    qtk_variant_t *v = qtk_variant_new(pid, name, strlen(name));
    if (!v) {
        qtk_variant_delete(v);
        return 0;
    }
    if (qtk_variant_prepare(v, DW->em, DW->ps, path, strlen(path))) {
        qtk_variant_kill(v, SIGKILL);
        if (!v->p.connected) qtk_variant_delete(v);
        return 0;
    }
    qtk_ictl_update_cnt(v->name.data, v->name.len, 1);
    lua_pushlightuserdata(L, v);
    v->used = 1;
    luaL_setmetatable(L, LUA_DLGHANDLE);
    return 1;
}

static int _raw_pend_continuation(lua_State *L, int status, lua_KContext ctx)
{
    size_t len;
    const char *res = luaL_checklstring(L, -1, &len);
    qtk_variant_t *v = _new_inst(res, len);
    if (!v) return 0;
    lua_pushlightuserdata(L, v);
    v->used = 1;
    return 1;
}

static int _check_raw_cnt(lua_State *L, const char *res, size_t l)
{
    int cnt = qtk_ictl_get_cnt_s("raw");
    cnt = cnt < 0 ? 0 : cnt;
    if (cnt > DW->cfg->max_raw_cnt) {
        int pending_cnt = qtk_ictl_get_pending_cnt_s("raw");
        if (pending_cnt > DW->cfg->max_pending) {
            qtk_debug("Raw Inst Too Busy\n");
            return -1;
        }
        int ref = qtk_lreg_yield_co(L, -1);
        char tmp[64];
        snprintf(tmp, sizeof(tmp), "%d", ref);
        wtk_string_t ud = {cast(char *, res), l};
        qtk_ictl_pend_s("raw", ref, &ud);
        return lua_yieldk(L, 0, 0, _raw_pend_continuation);
    }
    return 0;
}

static int _inst_pend_continuation(lua_State *L, int status, lua_KContext ctx)
{
    size_t len;
    const char *res = luaL_checklstring(L, -1, &len);
    qtk_variant_t *v = qtk_fty_pop(res, len);
    if (v == NULL) {
        qtk_variant_t *Z = qtk_incub_find(res, len);
        int ref = qtk_lreg_yield_co(L, 500);
        char tmp[64];
        snprintf(tmp, sizeof(tmp), "%d", ref);
        if (qtk_variant_write_cmd(Z, GETINST, tmp)) {
            qtk_variant_kill(Z, SIGKILL);
            qtk_lunreg_yield_co(L, ref);
            return 0;
        }
        return lua_yieldk(L, 0, 0, _new_continuation);     // waiting for zygote prepare inst
    }
    lua_pushlightuserdata(L, v);
    v->used = 1;
    luaL_setmetatable(L, LUA_DLGHANDLE);
    return 1;
}

static int _check_inst_cnt(lua_State *L, const char *n, size_t l)
{
    int cnt = qtk_ictl_get_cnt(n, l);
    cnt = cnt < 0 ? 0 : cnt;
    qtk_inst_cfg_t *cfg = qtk_dlg_worker_find_inst_cfg(DW->cfg, n, l);
    if (cnt > cfg->maxCnt) {
        int pending_cnt = qtk_ictl_get_pending_cnt(n, l);
        if (pending_cnt > DW->cfg->max_pending) {
            qtk_debug("%.*s Too Busy\n", cast(int, l), n);
            return -1;
        }
        int ref = qtk_lreg_yield_co(L, 500);
        wtk_string_t ud = {cast(char *, n), l};
        qtk_ictl_pend(n, l, ref, &ud);
        return lua_yieldk(L, 0, 0, _inst_pend_continuation);
    }
    return 0;
}

static int lnew(lua_State *L)
{
    const char *res;
    size_t len;
    res = luaL_checklstring(L, 1, &len);
    qtk_variant_t *v = qtk_fty_pop(res, len);
    if (NULL == v) {
        qtk_variant_t *Z = qtk_incub_find(res, len);
        if (NULL == Z) {
            if (_check_raw_cnt(L, res, len)) return 0;
            v = _new_inst(res, len);
            if (!v) return 0;
        } else {
            if (_check_inst_cnt(L, res, len)) return 0;
            int ref = qtk_lreg_yield_co(L, 500);
            char tmp[64];
            snprintf(tmp, sizeof(tmp), "%d", ref);
            if (qtk_variant_write_cmd(Z, GETINST, tmp)) {
                qtk_variant_kill(Z, SIGKILL);
                qtk_lunreg_yield_co(L, ref);
                return 0;
            }
            return lua_yieldk(L, 0, 0, _new_continuation);     // waiting for zygote prepare inst
        }
    }
    lua_pushlightuserdata(L, v);
    v->used = 1;
    luaL_setmetatable(L, LUA_DLGHANDLE);
    return 1;
}

static int lclear(lua_State *L)
{
    const char *res;
    size_t len;
    res = luaL_checklstring(L, 1, &len);
    qtk_fty_clear(res, len);
    qtk_incub_clear(res, len);
    return 0;
}

static const luaL_Reg mlib[] = {
    {"new", lnew},
    {"clear", lclear},
    {NULL, NULL}
};

LUAMOD_API int luaopen_dlg(lua_State *L)
{
    luaL_newlib(L, mlib);
    createmeta(L);
    lua_getfield(L, LUA_REGISTRYINDEX, "worker");
    DW = lua_touserdata(L, -1);
    lua_pop(L, 1);
    return 1;
}
