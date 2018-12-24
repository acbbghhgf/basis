#include "third/lua5.3.4/src/lua.h"
#include "third/lua5.3.4/src/lauxlib.h"
#include "third/uuid/uuid4.h"
#include "main/qtk_context.h"
#include "main/qtk_bifrost_core.h"
#include "main/qtk_bifrost.h"
#include "main/qtk_timer.h"
#include "os/qtk_base.h"
#include "os/qtk_alloc.h"
#include "lua_seri.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

static int traceback(lua_State *L)
{
    const char *msg = lua_tostring(L, 1);
    if (msg)
        luaL_traceback(L, L, msg, 1);
    else {
        lua_pushliteral(L, "(no error message)");
    }
    return 1;
}

static int _cb(qtk_context_t *ctx, void *ud, int type, int session, uint32_t source, const void *msg, size_t sz)
{
	lua_State *L = ud;
	int trace = 0;
	int r;
	int top = lua_gettop(L);
	if (top == 0) {
        lua_pushcfunction(L, traceback);
		lua_rawgetp(L, LUA_REGISTRYINDEX, _cb);
	} else {
		assert(top == 2);
	}
	lua_pushvalue(L,2);

	lua_pushinteger(L, type);
	lua_pushlightuserdata(L, (void *)msg);
	lua_pushinteger(L, sz);
	lua_pushinteger(L, session);
	lua_pushinteger(L, source);

	r = lua_pcall(L, 5, 0 , trace);

	if (r == LUA_OK) {
		return 0;
	}

	switch (r) {
	case LUA_ERRRUN:
        qtk_debug("lua call self = %x, error : %s\n", CTX_GET_HANDLE(ctx), lua_tostring(L, -1));
		break;
	case LUA_ERRMEM:
		qtk_debug("lua memory error : self = %x\n", CTX_GET_HANDLE(ctx));
		break;
	case LUA_ERRERR:
		qtk_debug("lua error : self = %x\n", CTX_GET_HANDLE(ctx));
		break;
	case LUA_ERRGCMM:
		qtk_debug("lua gc error : self = %x\n", CTX_GET_HANDLE(ctx));
		break;
	};

	lua_pop(L,1);

	return 0;
}


static int forward_cb(qtk_context_t *ctx, void *ud, int type, int session, uint32_t source, const void * msg, size_t sz) {
	_cb(ctx, ud, type, session, source, msg, sz);
	// don't delete msg in forward mode.
	return 1;
}


static int lcallback(lua_State *L) {
	qtk_context_t *ctx = lua_touserdata(L, lua_upvalueindex(1));
	int forward = lua_toboolean(L, 2);
	luaL_checktype(L, 1, LUA_TFUNCTION);
	lua_settop(L, 1);
	lua_rawsetp(L, LUA_REGISTRYINDEX, _cb);

	lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
	lua_State *gL = lua_tothread(L,-1);

	if (forward) {
		qtk_context_callback(ctx, forward_cb, (void *)gL);
	} else {
		qtk_context_callback(ctx, _cb, (void *)gL);
	}

	return 0;
}


static int lcommand(lua_State *L)
{
	qtk_context_t * ctx = lua_touserdata(L, lua_upvalueindex(1));
	const char *cmd = luaL_checkstring(L,1);
	const char *result;
	const char *param = NULL;
	if (lua_gettop(L) == 2) {
		param = luaL_checkstring(L,2);
	}

	result = qtk_command(ctx, cmd, param);
	if (result) {
		lua_pushstring(L, result);
		return 1;
	}
	return 0;
}


/* param and return value for qtk_command can be number */
static int lintcommand(lua_State *L)
{
	qtk_context_t * ctx = lua_touserdata(L, lua_upvalueindex(1));
	const char * cmd = luaL_checkstring(L,1);
	const char * result;
	const char * param = NULL;
	char tmp[64];
	if (lua_gettop(L) == 2) {
		if (lua_isnumber(L, 2)) {
			int32_t n = (int32_t)luaL_checkinteger(L,2);
			sprintf(tmp, "%d", n);
			param = tmp;
		} else {
			param = luaL_checkstring(L,2);
		}
	}

	result = qtk_command(ctx, cmd, param);
	if (result) {
		char *endptr = NULL;
		lua_Integer r = strtoll(result, &endptr, 0);
		if (endptr == NULL || *endptr != '\0') {
			// may be real number
			double n = strtod(result, &endptr);
			if (endptr == NULL || *endptr != '\0') {
				return luaL_error(L, "Invalid result %s", result);
			} else {
				lua_pushnumber(L, n);
			}
		} else {
			lua_pushinteger(L, r);
		}
		return 1;
	}
	return 0;
}


static int lgenid(lua_State *L)
{
	qtk_context_t *ctx = lua_touserdata(L, lua_upvalueindex(1));
	int session = qtk_bifrost_send(ctx, 0, 0, QTK_TAG_ALLOCSESSION | QTK_TAG_DONOTCOPY, 0 , NULL, 0);
	lua_pushinteger(L, session);
	return 1;
}


static int lgenUUID(lua_State *L)
{
    char buf[UUID4_LEN];
    uuid4_generate(buf);
    lua_pushstring(L, buf);
    return 1;
}


static const char *get_dest_string(lua_State *L, int index)
{
	const char *dest_string = lua_tostring(L, index);
	if (dest_string == NULL) {
		luaL_error(L, "dest address type (%s) must be a string or number.", lua_typename(L, lua_type(L,index)));
	}
	return dest_string;
}


/* param: dest string or number
 *        type (idx_type + 0)
 *        session number (idx_type + 1)
 *        msg_content string or lightuserdata (idx_type + 2)
 *        if msg_content == lightuserdata, len of lightuserdata (idx_type + 3)
 * */
static int send_message(lua_State *L, int source, int idx_type)
{
    size_t len = 0;
    void *msg = NULL;

	qtk_context_t * ctx = lua_touserdata(L, lua_upvalueindex(1));
	uint32_t dest = (uint32_t)lua_tointeger(L, 1);
	const char *dest_string = NULL;
	if (dest == 0) {
		if (lua_type(L,1) == LUA_TNUMBER) {
			return luaL_error(L, "Invalid service address 0");
		}
		dest_string = get_dest_string(L, 1);
	}

	int type = luaL_checkinteger(L, idx_type+0);
	int session = 0;
	if (lua_isnil(L,idx_type+1)) {
		type |= QTK_TAG_ALLOCSESSION;
	} else {
		session = luaL_checkinteger(L,idx_type+1);
	}

	int mtype = lua_type(L,idx_type+2);
	switch (mtype) {
    case LUA_TSTRING:
		msg = (void *)lua_tolstring(L,idx_type+2,&len);
		if (len == 0) {
			msg = NULL;
		}
		if (dest_string) {
			session = qtk_bifrost_sendname(ctx, source, dest_string, type, session , msg, len);
		} else {
			session = qtk_bifrost_send(ctx, source, dest, type, session , msg, len);
		}
		break;
	case LUA_TLIGHTUSERDATA:
		msg = lua_touserdata(L,idx_type+2);
		int size = luaL_checkinteger(L,idx_type+3);
		if (dest_string) {
			session = qtk_bifrost_sendname(ctx, source, dest_string, type | QTK_TAG_DONOTCOPY, session, msg, size);
		} else {
			session = qtk_bifrost_send(ctx, source, dest, type | QTK_TAG_DONOTCOPY, session, msg, size);
		}
		break;
	default:
		luaL_error(L, "invalid param %s", lua_typename(L, lua_type(L,idx_type+2)));
	}
	if (session < 0) {
		// TODO: maybe throw an error would be better
        qtk_debug("send message error\n");
		return 0;
	}
	lua_pushinteger(L,session);

	return 1;
}


static int lsend(lua_State *L)
{
	return send_message(L, 0, 2);
}


static int lredirect(lua_State *L)
{
	uint32_t source = (uint32_t)luaL_checkinteger(L,2);
	return send_message(L, source, 3);
}


static int lerror(lua_State *L)
{
	int n = lua_gettop(L);
	if (n <= 1) {
		lua_settop(L, 1);
		const char * s = luaL_tolstring(L, 1, NULL);
        qtk_debug("%s\n", s);
		return 0;
	}

	luaL_Buffer b;
	luaL_buffinit(L, &b);
	int i;
	for (i=1; i<=n; i++) {
		luaL_tolstring(L, i, NULL);
		luaL_addvalue(&b);
		if (i<n) {
			luaL_addchar(&b, ' ');
		}
	}
	luaL_pushresult(&b);
	qtk_debug("%s\n", lua_tostring(L, -1));

	return 0;
}


/* userdata to string */
static int ltostring(lua_State *L)
{
	if (lua_isnoneornil(L,1)) {
		return 0;
	}
	char * msg = lua_touserdata(L,1);
	int sz = luaL_checkinteger(L,2);
	lua_pushlstring(L,msg,sz);

	return 1;
}


static int ltrash(lua_State *L)
{
	int t = lua_type(L,1);
	switch (t) {
	case LUA_TSTRING: {
		break;
	}
	case LUA_TLIGHTUSERDATA: {
		void * msg = lua_touserdata(L,1);
		luaL_checkinteger(L,2);
		qtk_free(msg);
		break;
	}
	default:
		luaL_error(L, "bifrost.trash invalid param %s", lua_typename(L,t));
	}

	return 0;
}


static int lnow(lua_State *L)
{
	uint64_t ti = qtk_timer_now();
	lua_pushinteger(L, ti);
	return 1;
}


static int lstarttime(lua_State *L)
{
	uint64_t ti = qtk_timer_starttime();
	lua_pushinteger(L, ti);
	return 1;
}

static int ltime(lua_State *L)
{
    lua_Number epoch = lua_tonumber(L, 1);
    time_t time = epoch;
    struct tm ts;
    char buf[80];
    localtime_r(&time, &ts);
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S", &ts);
    lua_pushstring(L, buf);
    return 1;
}

static int lepoch(lua_State *L)
{
    uint64_t ti = qtk_timer_now();
    uint64_t startTi = qtk_timer_starttime();
    lua_pushinteger(L, startTi * 1000 + ti * 10);
    return 1;
}

LUAMOD_API int luaopen_bifrost_core(lua_State *L)
{
	luaL_checkversion(L);

	luaL_Reg l[] = {
		{ "send" , lsend },
		{ "genid", lgenid },
        { "genUUID", lgenUUID },
		{ "redirect", lredirect },
		{ "command" , lcommand },
		{ "intcommand", lintcommand },
		{ "error", lerror },
		{ "tostring", ltostring },
		{ "trash" , ltrash },
		{ "callback", lcallback },
        { "pack", luaseri_pack },
        { "unpack", luaseri_unpack },
		{ "now", lnow },
        { "starttime", lstarttime},
        { "time", ltime},
        { "epoch", lepoch},
		{ NULL, NULL },
	};

	luaL_newlibtable(L, l);

	lua_getfield(L, LUA_REGISTRYINDEX, "qtk_context");
	qtk_context_t *ctx = lua_touserdata(L,-1);
	if (ctx == NULL) {
		return luaL_error(L, "Init bifrost context first");
	}

	luaL_setfuncs(L,l,1);

	return 1;
}
