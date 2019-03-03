#include "third/lua5.3.4/src/lauxlib.h"
#include "third/lua5.3.4/src/lua.h"
#include "qtk/sframe/qtk_sframe.h"
#include "main/qtk_proxy.h"
#include "main/qtk_xfer.h"
#include "qtk/http/qtk_request.h"

#include <assert.h>

static qtk_sframe_method_t *method;

static int _method_atoi(const char *method, int len)
{
    switch(len) {
    case 3:
        if (0 == memcmp(method, "GET", 3))
            return HTTP_GET;
        if (0 == memcmp(method, "PUT", 3))
            return HTTP_PUT;
        break;
    case 4:
        if (0 == memcmp(method, "POST", 4))
            return HTTP_POST;
        if (0 == memcmp(method, "HEAD", 4))
            return HTTP_HEAD;
        break;
    case 6:
        if (0 == memcmp(method, "DELETE", 6))
            return HTTP_DELETE;
        break;
    default:
        return HTTP_UNKNOWN;
    }
    return HTTP_UNKNOWN;
}

/* @param 1 is method(string) or status(number)
 * @param 2 is content(table) for RESPONSE or url for REQUEST
 * @param 3 is content(table) for REQUEST or nil for RESPONSE
 * @return value true if xfer ok or nil
 * */
static int lxfer(lua_State *L)
{
    int ret, id;
    size_t len;
    qtk_sframe_msg_t *msg = NULL;
    if (LUA_TSTRING == lua_type(L, 1)) {
        luaL_checktype(L, 2, LUA_TSTRING);
        luaL_checktype(L, 3, LUA_TTABLE);
        const char *meth = lua_tolstring(L, 1, &len);
        int meth_num = _method_atoi(meth, len);
        if (HTTP_UNKNOWN == meth_num) goto err;
        const char *url = lua_tostring(L, 2);
        lua_pushliteral(L, "addr");
        ret = lua_gettable(L, -2);
        if (ret != LUA_TNUMBER) {
            lua_pop(L, 1);
            goto err;
        }
        id = lua_tonumber(L, -1);
        lua_pop(L, 1);
        msg = method->new(method, QTK_SFRAME_MSG_REQUEST, id);
        method->set_url(msg, url, strlen(url));
        method->set_method(msg, meth_num);
    } else if (LUA_TNUMBER == lua_type(L, 1)) {
        int status = lua_tonumber(L, 1);
        luaL_checktype(L, 2, LUA_TTABLE);
        lua_pushliteral(L, "addr");
        ret = lua_gettable(L, -2);
        if (ret != LUA_TNUMBER) {
            lua_pop(L, 1);
            goto err;
        }
        id = lua_tonumber(L, -1);
        lua_pop(L, 1);
        msg = method->new(method, QTK_SFRAME_MSG_RESPONSE, id);
        method->set_status(msg, status);
    } else{
        goto err;
    }
    lua_pushliteral(L, "body");
    ret = lua_gettable(L, -2);
    if (ret == LUA_TSTRING) {
        const char *body = lua_tolstring(L, -1, &len);
        method->add_body(msg, body, len);
    }
    lua_pop(L, 1);
    lua_pushliteral(L, "field");
    ret = lua_gettable(L, -2);
    if (ret == LUA_TTABLE) {
        lua_pushnil(L);
        while(lua_next(L, -2)) {
            const char *key = lua_tostring(L, -2);
            const char *value = lua_tostring(L, -1);
            if (key && value)
                method->add_header(msg, key, value);
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    method->push(method, msg);
    lua_pushboolean(L, 1);
    return 1;
err:
    printf("xfer error\n");
    return 0;
}

/* @param 1 is addr we request to establish
 * @return value is id
 * */
static int lestb(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TSTRING);
    const char *addr = lua_tostring(L, 1);
    int id = qtk_xfer_estab(method, addr, 0);
    if (id < 0) return 0;
    lua_pushinteger(L, id);
    return 1;
}

/* @param 1 is id
 * @return value is true is push succ or nil
 * */
static int ldestroy(lua_State *L)
{
    lua_Integer id = luaL_checkinteger(L, 1);
    qtk_xfer_destroy(method, id);
    lua_pushboolean(L, 1);
    return 1;
}

LUAMOD_API int luaopen_xfer_core(lua_State *L)
{
	luaL_checkversion(L);

	luaL_Reg l[] = {
		{ "xfer"    , lxfer },
		{ "estb"    , lestb },
        { "destroy" , ldestroy },
        { NULL      , NULL },
	};
    assert(method == NULL);
	luaL_newlib(L, l);
    lua_getfield(L, LUA_REGISTRYINDEX, "worker");
    qtk_proxy_worker_t *worker = lua_touserdata(L, -1);
    method = ((qtk_proxy_t *)worker->uplayer)->gate;
    if (worker == NULL) {
        return luaL_error(L, "Must have worker");
    }

    lua_pop(L, 1);

	return 1;
}
