#include "third/lua5.3.4/src/lua.h"
#include "third/lua5.3.4/src/lualib.h"
#include "third/lua5.3.4/src/lauxlib.h"

#include "main/qtk_context.h"
#include "main/qtk_bifrost.h"
#include "main/qtk_bifrost_core.h"
#include "main/qtk_env.h"
#include "os/qtk_base.h"
#include "os/qtk_alloc.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MEMORY_WARNING_REPORT (1024 * 1024 * 32)

struct blua {
	lua_State * L;
	qtk_context_t * ctx;
	size_t mem;
	size_t mem_report;
	size_t mem_limit;
};


static int traceback (lua_State *L)
{
	const char *msg = lua_tostring(L, 1);
	if (msg)
		luaL_traceback(L, L, msg, 1);
	else {
		lua_pushliteral(L, "(no error message)");
	}
	return 1;
}


static void report_launcher_error(qtk_context_t *ctx)
{
	qtk_bifrost_sendname(ctx, 0, "launcher", QTK_ERROR, 0, "ERROR", 5);
}


static int init_cb(struct blua *l, qtk_context_t *ctx, const char *args, size_t sz)
{
	lua_State *L = l->L;
	l->ctx = ctx;
	lua_gc(L, LUA_GCSTOP, 0);
	luaL_openlibs(L);
	lua_pushlightuserdata(L, ctx);
	lua_setfield(L, LUA_REGISTRYINDEX, "qtk_context");

	/*const char *path = "./service/?.lua;./lualib/?.lua;./lualib/?/init.lua";*/
	const char *path = "service/?.lua;lualib/?.lua;lualib/?/init.lua";
	lua_pushstring(L, path);
	lua_setglobal(L, "LUA_PATH");
	const char *cpath = "./luaclib/?.so";
	lua_pushstring(L, cpath);
	lua_setglobal(L, "LUA_CPATH");
    const char *ppath = qtk_optstring("serviceBinPath", "");
	lua_pushstring(L, ppath);
	lua_setglobal(L, "LUA_PPATH");
	const char *service = "service/?.lua";
	lua_pushstring(L, service);
	lua_setglobal(L, "LUA_SERVICE");

	lua_pushcfunction(L, traceback);
	assert(lua_gettop(L) == 1);

    char loader[256];
    snprintf(loader, sizeof(loader), "%s&lualib/loader.lua", ppath);

	int r = luaL_loadfileq(L, loader);
	if (r != LUA_OK) {
		qtk_debug("Can't load %s : %s\n", loader, lua_tostring(L, 0));
        lua_pop(L, 1);
        r = luaL_loadfile(L, "lualib/loader.lua");
        if (r != LUA_OK) {
            qtk_debug("Retry failed: %s\n", lua_tostring(L, -1));
            return 1;
        }
	}
	lua_pushlstring(L, args, sz);
	r = lua_pcall(L,1,0,1);
	if (r != LUA_OK) {
		qtk_debug("lua loader error : %s\n", lua_tostring(L, -1));
		report_launcher_error(ctx);
		return 1;
	}
	lua_settop(L,0);
	if (lua_getfield(L, LUA_REGISTRYINDEX, "memlimit") == LUA_TNUMBER) {
		size_t limit = lua_tointeger(L, -1);
		l->mem_limit = limit;
		qtk_debug("Set memory limit to %.2f M\n", (float)limit / (1024 * 1024));
		lua_pushnil(L);
		lua_setfield(L, LUA_REGISTRYINDEX, "memlimit");
	}
	lua_pop(L, 1);

    lua_gc(L, LUA_GCRESTART, 0);

	return 0;
}

static int launch_cb(qtk_context_t * context, void *ud, int type, int session, uint32_t source , const void * msg, size_t sz)
{
	assert(type == QTK_SYS && session == 0);
	struct blua *l = ud;
	qtk_context_callback(context, NULL, NULL);
	int err = init_cb(l, context, msg, sz);
	if (err) {
		qtk_command(context, "EXIT", NULL);
	}

	return 0;
}

int blua_init(struct blua *l, qtk_context_t *ctx, const char * args)
{
	int sz = strlen(args);
	char *tmp = qtk_malloc(sz);
	memcpy(tmp, args, sz);
	qtk_context_callback(ctx, launch_cb, l);
    /*it must be first message*/
	qtk_bifrost_send(ctx, 0, CTX_GET_HANDLE(ctx), QTK_SYS | QTK_TAG_DONOTCOPY, 0, tmp, sz);

	return 0;
}

static void * lalloc(void * ud, void *ptr, size_t osize, size_t nsize)
{
	struct blua *l = ud;
	size_t mem = l->mem;
	l->mem += nsize;
	if (ptr)
		l->mem -= osize;
	if (l->mem_limit != 0 && l->mem > l->mem_limit) {
		if (ptr == NULL || nsize > osize) {
			l->mem = mem;
			return NULL;
		}
	}
	if (l->mem > l->mem_report) {
		l->mem_report *= 2;
		qtk_debug("Memory warning %.2f M", (float)l->mem / (1024 * 1024));
	}
	return qtk_lalloc(ptr, osize, nsize);
}

struct blua * blua_create(void)
{
	struct blua * l = qtk_malloc(sizeof(*l));
	memset(l,0,sizeof(*l));
	l->mem_report = MEMORY_WARNING_REPORT;
	l->mem_limit = 0;
	l->L = lua_newstate(lalloc, l);
	return l;
}

void blua_release(struct blua *l)
{
    lua_close(l->L);
    qtk_free(l);
}

void blua_signal(struct blua *l, int signal)
{
	qtk_debug("recv a signal :%d\n", signal);
}
