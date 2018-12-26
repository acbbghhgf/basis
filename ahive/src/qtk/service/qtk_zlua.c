#include "qtk/zeus/qtk_zeus.h"
#include "wtk/core/wtk_alloc.h"
#include "qtk/zeus/qtk_zeus_server.h"
#include <assert.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"


#ifdef LUA_CACHELIB
#define codecache luaopen_cache
#else
static int codecache_dummy(lua_State *L) {
    return 0;
}
static int luaopen_cache(lua_State *L) {
    luaL_Reg l[] = {
        { "clear", codecache_dummy },
        { "mode", codecache_dummy },
        { NULL, NULL },
    };
    luaL_newlib(L,l);
    lua_getglobal(L, "loadfile");
    lua_setfield(L, -2, "loadfile");
    return 1;
}
#endif


typedef struct qtk_zlua qtk_zlua_t;

struct qtk_zlua {
    lua_State *L;
    size_t mem;
    size_t mem_limit;
    size_t mem_report;
    struct qtk_zcontext *ctx;
};


static void* lalloc(void *ud, void *ptr, size_t osize, size_t nsize) {
    qtk_zlua_t *zlua = ud;
    size_t mem = zlua->mem;
    zlua->mem += nsize;
    if (ptr) {
        zlua->mem -= osize;
    }
    if (zlua->mem_limit != 0 && zlua->mem > zlua->mem_limit) {
        if (ptr == NULL || nsize > osize) {
            zlua->mem = mem;
            return NULL;
        }
    }
    if (zlua->mem > zlua->mem_report) {
        const char *name = qtk_zcontext_name(zlua->ctx);
        zlua->mem_report *= 2;
        if (name) {
            qtk_zerror(zlua->ctx, "[%s] Memory warning %.2f M", name,
                       (float)zlua->mem/(1024*1024));
        } else {
            qtk_zerror(zlua->ctx, "[%p] Memory warning %.2f M", zlua->ctx,
                       (float)zlua->mem/(1024*1024));
        }
    }
    if (0 == nsize) {
        wtk_free(ptr);
    } else {
        return realloc(ptr, nsize);
    }
    return NULL;
}


qtk_zlua_t* zlua_create(void) {
    qtk_zlua_t *inst = (qtk_zlua_t*)wtk_calloc(1, sizeof(*inst));
    inst->mem_report = 32 * 1024 * 1024;
    inst->L = lua_newstate(lalloc, inst);
    return inst;
}


void zlua_release(qtk_zlua_t *zlua) {
    if (zlua->L) {
        lua_close(zlua->L);
    }
    wtk_free(zlua);
}


static int traceback(lua_State *L) {
    const char *msg = lua_tostring(L, 1);
    if (msg) {
        luaL_traceback(L, L, msg, 1);
    } else {
        lua_pushliteral(L, "(no error message)");
    }
    return 1;
}


static int report_launcher_error(struct qtk_zcontext *ctx) {
    return qtk_zsendname(ctx, 0, ".launcher", ZEUS_PTYPE_TEXT, 0, "ERROR", 5);
}


static int init_cb(qtk_zlua_t *l, struct qtk_zcontext *ctx, const char *arg, size_t sz) {
    lua_State *L = l->L;
    l->ctx = ctx;
    lua_gc(L, LUA_GCSTOP, 0);
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
    luaL_openlibs(L);
    lua_pushlightuserdata(L, ctx);
    lua_setfield(L, LUA_REGISTRYINDEX, "qtk_zcontext");
    luaL_requiref(L, "zeus.codecache", codecache, 0);
    lua_pop(L, 1);

    const char *path = "./?.lua;../lualib/?.lua;../ext/lua/?.lua";
    lua_pushstring(L, path);
    lua_setglobal(L, "LUA_PATH");
    const char *cpath = "./?.so;../luaclib?.so";
    lua_pushstring(L, cpath);
    lua_setglobal(L, "LUA_CPATH");
    const char *service = "../service;../script";
    lua_pushstring(L, service);
    lua_setglobal(L, "LUA_SERVICE");

    lua_pushcfunction(L, traceback);
    assert(lua_gettop(L) == 1);

    const char *loader = "../lualib/loader.lua";
    int r = luaL_loadfile(L, loader);
    if (LUA_OK != r) {
        qtk_zerror(ctx, "Cannot load %s : %s", loader, lua_tostring(L, -1));
        report_launcher_error(ctx);
        return -1;
    }
    lua_pushlstring(L, arg, sz);
    r = lua_pcall(L, 1, 0, 1);
    if (LUA_OK != r) {
        qtk_zerror(ctx, "lua load error : %s", lua_tostring(L, -1));
        report_launcher_error(ctx);
        return -1;
    }
    lua_settop(L, 0);
    lua_gc(L, LUA_GCRESTART, 0);

    return 0;
}


static int launch_cb(struct qtk_zcontext *ctx, void *ud, int type, int session, uint32_t src,
                     const char *data, size_t sz) {
    assert(0 == type && 0 == session);
    struct qtk_zlua *l = ud;
    qtk_zcallback(ctx, NULL, NULL);
    if (init_cb(l, ctx, data, sz)) {
        qtk_zcommand(ctx, "EXIT", NULL);
    }
    return 0;
}


int zlua_init(qtk_zlua_t *l, struct qtk_zcontext *ctx, const char *param) {
    qtk_zcallback(ctx, l, launch_cb);
    const char *self = qtk_zcommand(ctx, "REG", NULL);
    uint32_t handle = strtol(self+1, NULL, 16);
    qtk_zsend(ctx, 0, handle, 0, 0, (void*)param, strlen(param));
    return 0;
}
