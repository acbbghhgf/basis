#include "third/lua5.3.4/src/lua.h"
#include "third/lua5.3.4/src/lauxlib.h"
#include "third/lua5.3.4/src/lualib.h"
#include "main/qtk_dlg_worker.h"
#include "qtk/os/qtk_timer.h"
#include "qtk/os/qtk_base.h"
#include "third/uuid/uuid4.h"
#include "qtk_lco.h"

#include <assert.h>

static const char *handle = "YIELD_CO";

static void _co_timeout(void *ud)
{
    int ref = (long long)ud;
    qtk_dlg_worker_t *dw = qtk_dlg_worker_self();
    lua_State *co = qtk_lget_yield_co(dw->L, ref);
    if (co == NULL) {
        lua_pop(dw->L, 1);
        return ;
    }
    lua_pushinteger(co, -1);
    int ret = lua_resume(co, NULL, 1);
    if (ret > LUA_YIELD) {
        qtk_debug("%d\t%s\n", ret, lua_tostring(co, -1));
    }
    qtk_lunreg_yield_co(dw->L, ref);
    lua_pop(dw->L, 1);
}

int qtk_lreg_yield_co(lua_State *L, int timeout)
{
    static int ref = 0;
    luaL_newmetatable(L, handle);
    lua_pushthread(L);
    lua_rawseti(L, -2, ref);
    int ret = ref;
    ref ++;
    lua_pop(L, 1);

    if (timeout >= 0)
        qtk_timer_add(timeout, _co_timeout, (void *)(long long)ret);

    return ret;
}

int qtk_lunreg_yield_co(lua_State *L, int ref)
{
    luaL_newmetatable(L, handle);
    lua_pushnil(L);
    lua_rawseti(L, -2, ref);
    lua_pop(L, 1);
    return 0;
}

/* return value will at top of stack */
lua_State *qtk_lget_yield_co(lua_State *L, int ref)
{
    lua_State *ret = NULL;
    luaL_newmetatable(L, handle);
    int type = lua_rawgeti(L, -1, ref);
    lua_replace(L, -2);
    if (type == LUA_TTHREAD)
        ret = lua_tothread(L, -1);
    else {
        assert(type == LUA_TNIL);
    }
    return ret;
}
