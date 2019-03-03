#include "third/lua5.3.4/src/lauxlib.h"
#include "third/lua5.3.4/src/lua.h"
#include "main/qtk_proxy_worker.h"
#include "third/uuid/uuid4.h"
#include "main/qtk_timer.h"
#include "os/qtk_alloc.h"

#include <assert.h>

struct lua_function_wrapper {
    int function_ref;
    lua_State *L;
};

static struct lua_function_wrapper *_wrap_lua_function(int ref, lua_State *L)
{
    struct lua_function_wrapper *wrapper = qtk_calloc(1, sizeof(*wrapper));
    if (NULL == wrapper) return NULL;
    wrapper->function_ref = ref;
    wrapper->L = L;
    return wrapper;
}

static void _lua_function_unwrap(struct lua_function_wrapper *wrapper)
{
    lua_State *L = wrapper->L;
    int ret = lua_rawgeti(L, LUA_REGISTRYINDEX, wrapper->function_ref);
    if (LUA_TFUNCTION != ret) goto end;
    ret = lua_pcall(L, 0, 0, 0);
    if (ret != LUA_OK) printf("error execute");
end:
    luaL_unref(L, LUA_REGISTRYINDEX, wrapper->function_ref);
    qtk_free(wrapper);
}

static void _lua_function_excute(struct lua_function_wrapper *wrapper)
{
    _lua_function_unwrap(wrapper);
}

static int lgenUUID(lua_State *L)
{
    char buf[UUID4_LEN];
    uuid4_generate(buf);
    lua_pushstring(L, buf);
    return 1;
}

static int lnow(lua_State *L)
{
    uint64_t ti = qtk_timer_now();
    lua_pushinteger(L, ti);
    return 1;
}

/* param 1 is timeout
 * param 2 is handler */
static int ltimeout(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TNUMBER);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    int ti = lua_tointeger(L, 1);
    lua_pushvalue(L, 2);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
    lua_State *gL = lua_tothread(L, -1);
    lua_pop(L, 1);
    struct lua_function_wrapper *wrapper = _wrap_lua_function(ref, gL);
    if (NULL == wrapper) return 0;
    qtk_timer_add(ti, (timer_execute_func)_lua_function_excute, wrapper);
    lua_pushboolean(L, 1);
    return 1;
}

LUAMOD_API int luaopen_proxy_core(lua_State *L)
{
	luaL_checkversion(L);

	luaL_Reg l[] = {
		{ "genUUID" , lgenUUID },
        { "timeout" , ltimeout},
        { "now"     , lnow },
        { NULL      , NULL },
	};
	luaL_newlib(L, l);

    lua_getfield(L, LUA_REGISTRYINDEX, "worker");
    qtk_proxy_worker_t *worker = lua_touserdata(L, -1);
    if (worker == NULL) {
        return luaL_error(L, "Must have worker");
    }

    lua_pop(L, 1);

	return 1;
}
