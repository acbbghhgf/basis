#include "third/lua5.3.4/src/lua.h"
#include "third/lua5.3.4/src/lauxlib.h"

#include "qtk/sframe/qtk_sframe.h"
#include "main/qtk_dlg_worker.h"
#include "main/qtk_dlg.h"
#include "main/qtk_xfer.h"

static qtk_dlg_worker_t *DW;
#define GET_METHOD()  DW->dlg->gate

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
    printf("%s\n", method);
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
    qtk_sframe_method_t *method = GET_METHOD();
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
            printf("req addr not number\n");
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
            printf("rep addr not number, %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
            goto err;
        }
        id = lua_tonumber(L, -1);
        if (id < 0) goto err;
        lua_pop(L, 1);
        msg = method->new(method, QTK_SFRAME_MSG_RESPONSE, id);
        method->set_status(msg, status);
    } else{
        printf("err 1st param type\n");
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

static int _recon_atoi(const char *recon, int *succ)
{
    *succ = 1;
    if (0 == strcmp(recon, "LAZY_RECONNECT"))
        return QTK_SFRAME_RECONNECT_LAZY;
    if (0 == strcmp(recon, "NOT_RECONNECT"))
        return QTK_SFRAME_NOT_RECONNECT;
    if (0 == strcmp(recon, "NOW_RECONNECT"))
        return QTK_SFRAME_RECONNECT_NOW;
    *succ = 0;
    return 0;
}

/* @param 1 is addr we request to establish
 * @param 2 is reconnect(string)
 * @return value is id
 * */
static int lestb(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TSTRING);
    const char *addr = lua_tostring(L, 1);
    const char *recon = lua_tostring(L, 2);
    int succ;
    int recon_num = _recon_atoi(recon, &succ);
    if (!succ) return 0;
    qtk_sframe_method_t *method = GET_METHOD();
    int id = qtk_xfer_estab(method, addr, recon_num);
    if (id < 0) return 0;
    lua_pushinteger(L, id);
    return 1;
}

static int llisten(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TSTRING);
    int backlog = luaL_checkinteger(L, 2);
    const char *addr = lua_tostring(L, 1);
    qtk_sframe_method_t *method = GET_METHOD();
    int id = qtk_xfer_listen(method, addr, backlog);
    if (id < 0) return 0;
    lua_pushinteger(L, id);
    return 1;
}

static int lnewPool(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TSTRING);
    int nslot = luaL_checkinteger(L, 2);
    const char *addr = lua_tostring(L, 1);
    qtk_sframe_method_t *method = GET_METHOD();
    int id = qtk_xfer_newPool(method, addr, nslot);
    if (id < 0) return 0;
    lua_pushinteger(L, -id);
    return 1;
}

/* @param 1 is addr
 * @return value is true is push succ or nil
 * */
static int ldestroy(lua_State *L)
{
    lua_Integer id = luaL_checkinteger(L, 1);
    qtk_sframe_method_t *method = GET_METHOD();
    qtk_xfer_destroy(method, id);
    lua_pushboolean(L, 1);
    return 1;
}

LUAMOD_API int luaopen_xfer(lua_State *L)
{
	luaL_checkversion(L);

	luaL_Reg l[] = {
		{ "xfer"    , lxfer },
		{ "estb"    , lestb },
        { "destroy" , ldestroy },
        { "listen"  , llisten},
        { "newPool" , lnewPool},
        { NULL      , NULL },
	};
	luaL_newlib(L, l);
    lua_getfield(L, LUA_REGISTRYINDEX, "worker");
    DW = lua_touserdata(L, -1);
    lua_pop(L, 1);
	return 1;
}
