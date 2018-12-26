#include "qtk_zenv.h"
#include "lauxlib.h"
#include <assert.h>


qtk_zenv_t* qtk_zenv_new() {
    qtk_zenv_t *zenv = wtk_malloc(sizeof(*zenv));
    wtk_lock_init(&zenv->lock);
    zenv->L = luaL_newstate();
    return zenv;
}


int qtk_zenv_delete(qtk_zenv_t* zenv) {
    if (zenv->L) {
        lua_close(zenv->L);
        zenv->L = NULL;
    }
    wtk_lock_init(&zenv->lock);
    wtk_free(zenv);
    return 0;
}


const char* qtk_zenv_get(qtk_zenv_t *zenv, const char *key) {
    wtk_lock_lock(&zenv->lock);
    lua_State *L = zenv->L;
    lua_getglobal(L, key);
    const char *res = lua_tostring(L, -1);
    lua_pop(L, 1);
    wtk_lock_unlock(&zenv->lock);
    return res;
}


void qtk_zenv_set(qtk_zenv_t *zenv, const char *key, const char *value) {
    wtk_lock_lock(&zenv->lock);
    lua_State *L = zenv->L;
    lua_getglobal(L, key);
    assert(lua_isnil(L, -1));
    lua_pop(L, 1);
    lua_pushstring(L, value);
    lua_setglobal(L, key);
    wtk_lock_unlock(&zenv->lock);
}
