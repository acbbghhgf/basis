#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"


static int lsend(lua_State* L) {
    return 0;
}


LUAMOD_API int luaopen_zeus_core(lua_State *L) {
    luaL_checkversion(L);

    luaL_Reg l[] = {
        { "send" , lsend },
        { NULL, NULL },
    };

    luaL_newlibtable(L, l);

    lua_getfield(L, LUA_REGISTRYINDEX, "qtk_zcontext");
    struct qtk_zcontext *ctx = lua_touserdata(L,-1);
    if (ctx == NULL) {
        return luaL_error(L, "Init skynet context first");
    }

    luaL_setfuncs(L,l,1);

    return 1;
}
