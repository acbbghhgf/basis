#include "wtk/core/wtk_os.h"
#include "qtk/zeus/qtk_zeus.h"
#include "qtk_serialize.h"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"

#include <lua.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include "wtk/core/wtk_alloc.h"
#include "wtk/os/wtk_cpu.h"
#include "qtk/zeus/qtk_zeus_server.h"

/* int qtk_zsend(struct qtk_zcontext *ctx, uint32_t src, uint32_t dst, int type,
                     int session, void *data, size_t sz) { return 0; }
int qtk_zsendname(struct qtk_zcontext *ctx, uint32_t src, const char *addr, int type,
                  int session, void *data, size_t sz) {return 0;}
void qtk_zerror(struct qtk_zcontext *ctx, const char *msg, ...) {}
void qtk_zcallback(struct qtk_zcontext *ctx, void *ud, qtk_zeus_cb cb){}
const char* qtk_zcommand(struct qtk_zcontext *ctx, const char *cmd, const char *param){return NULL;}
uint64_t qtk_zeus_now(){return 0;}*/


struct snlua {
	lua_State * L;
	struct qtk_zcontext * ctx;
	const char * preload;
};

static int traceback (lua_State *L) {
	const char *msg = lua_tostring(L, 1);
	if (msg)
		luaL_traceback(L, L, msg, 1);
	else {
		lua_pushliteral(L, "(no error message)");
	}
	return 1;
}

static int _cb(struct qtk_zcontext * context, void * ud, int type, int session,
               uint32_t source, const char * msg, size_t sz) {
	lua_State *L = ud;
	int trace = 1;
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
	lua_pushinteger(L,sz);
	lua_pushinteger(L, session);
	lua_pushinteger(L, source);

	r = lua_pcall(L, 5, 0 , trace);

	if (r == LUA_OK) {
		return 0;
	}
	const char * self = qtk_zcommand(context, "REG", NULL);
	switch (r) {
	case LUA_ERRRUN:
		qtk_zerror(context, "lua call [%x to %s : %d msgsz = %d] error : " KRED "%s" KNRM, source , self, session, sz, lua_tostring(L,-1));
		break;
	case LUA_ERRMEM:
		qtk_zerror(context, "lua memory error : [%x to %s : %d]", source , self, session);
		break;
	case LUA_ERRERR:
		qtk_zerror(context, "lua error in error : [%x to %s : %d]", source , self, session);
		break;
	case LUA_ERRGCMM:
		qtk_zerror(context, "lua gc error : [%x to %s : %d]", source , self, session);
		break;
	case LUA_ERRSYNTAX:
		qtk_zerror(context, "lua syntax error : [%x to %s : %d]", source , self, session);
		break;
    case LUA_YIELD:
        wtk_debug("lua yield: [%x to %s : %d]\r\n", source, self, session);
        qtk_zserver_log("lua yield: [%x to %s : %d]", source, self, session);
        break;
	};

	lua_pop(L,1);

	return 0;
}

static int forward_cb(struct qtk_zcontext * context, void * ud, int type, int session,
                      uint32_t source, const char * msg, size_t sz) {
	_cb(context, ud, type, session, source, msg, sz);
	// don't delete msg in forward mode.
	return 1;
}

static int lcallback(lua_State *L) {
	struct qtk_zcontext * context = lua_touserdata(L, lua_upvalueindex(1));
	int forward = lua_toboolean(L, 2);
	luaL_checktype(L,1,LUA_TFUNCTION);
	lua_settop(L,1);
	lua_rawsetp(L, LUA_REGISTRYINDEX, _cb);

	lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
	lua_State *gL = lua_tothread(L,-1);

	if (forward) {
		qtk_zcallback(context, gL, forward_cb);
	} else {
		qtk_zcallback(context, gL, _cb);
	}

	return 0;
}

static int lcommand(lua_State *L) {
	struct qtk_zcontext * context = lua_touserdata(L, lua_upvalueindex(1));
	const char * cmd = luaL_checkstring(L,1);
	const char * result;
	const char * parm = NULL;
	if (lua_gettop(L) == 2) {
		parm = luaL_checkstring(L,2);
	}

	result = qtk_zcommand(context, cmd, parm);
	if (result) {
		lua_pushstring(L, result);
		return 1;
	}
	return 0;
}

static int lintcommand(lua_State *L) {
	struct qtk_zcontext * context = lua_touserdata(L, lua_upvalueindex(1));
	const char * cmd = luaL_checkstring(L,1);
	const char * result;
	const char * parm = NULL;
	char tmp[64];	// for integer parm
	if (lua_gettop(L) == 2) {
		if (lua_isnumber(L, 2)) {
			int32_t n = (int32_t)luaL_checkinteger(L,2);
			sprintf(tmp, "%d", n);
			parm = tmp;
		} else {
			parm = luaL_checkstring(L,2);
		}
	}

	result = qtk_zcommand(context, cmd, parm);
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

static int lgenid(lua_State *L) {
	struct qtk_zcontext * context = lua_touserdata(L, lua_upvalueindex(1));
	int session = qtk_zsend(context, 0, 0, ZEUS_PTYPE_TAG_ALLOCSESSION , 0 , NULL, 0);
	lua_pushinteger(L, session);
	return 1;
}

static const char * get_dest_string(lua_State *L, int index) {
	const char * dest_string = lua_tostring(L, index);
	if (dest_string == NULL) {
		luaL_error(L, "dest address type (%s) must be a string or number.", lua_typename(L, lua_type(L,index)));
	}
	return dest_string;
}

static int send_message(lua_State *L, int source, int idx_type) {
	struct qtk_zcontext * context = lua_touserdata(L, lua_upvalueindex(1));
	uint32_t dest = (uint32_t)lua_tointeger(L, 1);
	const char * dest_string = NULL;
	if (dest == 0) {
		if (lua_type(L,1) == LUA_TNUMBER) {
			return luaL_error(L, "Invalid service address 0");
		}
		dest_string = get_dest_string(L, 1);
	}

	int type = luaL_checkinteger(L, idx_type+0);
	int session = 0;
	if (lua_isnil(L,idx_type+1)) {
		type |= ZEUS_PTYPE_TAG_ALLOCSESSION;
	} else {
		session = luaL_checkinteger(L,idx_type+1);
	}

	int mtype = lua_type(L,idx_type+2);
	switch (mtype) {
	case LUA_TSTRING: {
		size_t len = 0;
		void * msg = (void *)lua_tolstring(L,idx_type+2,&len);
		if (len == 0) {
			msg = NULL;
		}
		if (dest_string) {
			session = qtk_zsendname(context, source, dest_string, type, session , msg, len);
		} else {
			session = qtk_zsend(context, source, dest, type, session , msg, len);
		}
		break;
	}
	case LUA_TLIGHTUSERDATA: {
		void * msg = lua_touserdata(L,idx_type+2);
		int size = luaL_checkinteger(L,idx_type+3);
		if (dest_string) {
			session = qtk_zsendname(context, source, dest_string,
                                    type | ZEUS_PTYPE_TAG_DONTCOPY, session, msg, size);
		} else {
			session = qtk_zsend(context, source, dest, type | ZEUS_PTYPE_TAG_DONTCOPY,
                                session, msg, size);
		}
		break;
	}
	default:
		luaL_error(L, "invalid param %s", lua_typename(L, lua_type(L,idx_type+2)));
	}
	if (session < 0) {
		// send to invalid address
		// todo: maybe throw an error would be better
		return 0;
	}
	lua_pushinteger(L,session);
	return 1;
}

/*
	uint32 address
	 string address
	integer type
	integer session
	string message
	 lightuserdata message_ptr
	 integer len
 */
static int lsend(lua_State *L) {
	return send_message(L, 0, 2);
}

/*
	uint32 address
	 string address
	integer source_address
	integer type
	integer session
	string message
	 lightuserdata message_ptr
	 integer len
 */
static int lredirect(lua_State *L) {
	uint32_t source = (uint32_t)luaL_checkinteger(L,2);
	return send_message(L, source, 3);
}

static int lerror(lua_State *L) {
	struct qtk_zcontext * context = lua_touserdata(L, lua_upvalueindex(1));
	int n = lua_gettop(L);
	if (n <= 1) {
		lua_settop(L, 1);
		const char * s = luaL_tolstring(L, 1, NULL);
		qtk_zerror(context, "%s", s);
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
	qtk_zerror(context, "%s", lua_tostring(L, -1));
	return 0;
}

static int ltostring(lua_State *L) {
	if (lua_isnoneornil(L,1)) {
		return 0;
	}
	char * msg = lua_touserdata(L,1);
	int sz = luaL_checkinteger(L,2);
	lua_pushlstring(L,msg,sz);
	return 1;
}

static int ltoint(lua_State *L) {
	if (lua_isnoneornil(L,1)) {
		return 0;
	}
    int ret;
    char *msg = lua_touserdata(L, 1);
    assert(lua_tointeger(L, 2) >= sizeof(int));
    memcpy(&ret, msg, sizeof(int));
    ret = ntohl(ret);
    lua_pushinteger(L, ret);
    return 1;
}

static int lpackstring(lua_State *L) {
	qtk_lua_pack(L);
	char * str = (char *)lua_touserdata(L, -2);
	int sz = lua_tointeger(L, -1);
	lua_pushlstring(L, str, sz);
	wtk_free(str);
	return 1;
}

static int ltrash(lua_State *L) {
	int t = lua_type(L,1);
	switch (t) {
	case LUA_TSTRING: {
		break;
	}
	case LUA_TLIGHTUSERDATA: {
		void * msg = lua_touserdata(L,1);
		luaL_checkinteger(L,2);
		wtk_free(msg);
		break;
	}
	default:
		luaL_error(L, "skynet.trash invalid param %s", lua_typename(L,t));
	}

	return 0;
}

static int lnow(lua_State *L) {
	uint64_t ti = qtk_zeus_now();
	lua_pushinteger(L, ti);
	return 1;
}

static int lrawnow(lua_State *L) {
    uint64_t ti = (uint64_t)time_get_ms();
	lua_pushinteger(L, ti);
	return 1;
}

static int lgetcpus(lua_State *L) {
    lua_pushinteger(L, wtk_get_cpus());
    return 1;
}

LUAMOD_API int luaopen_zeus_core(lua_State *L) {
	luaL_checkversion(L);

	luaL_Reg l[] = {
		{ "send" , lsend },
		{ "genid", lgenid },
		{ "redirect", lredirect },
		{ "command" , lcommand },
		{ "intcommand", lintcommand },
		{ "error", lerror },
		{ "tostring", ltostring },
		{ "toint", ltoint },
		{ "pack", qtk_lua_pack },
		{ "unpack", qtk_lua_unpack },
		{ "packstring", lpackstring },
		{ "trash" , ltrash },
		{ "callback", lcallback },
		{ "now", lnow },
		{ "rawnow", lrawnow },
        { "getcpus", lgetcpus },
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
