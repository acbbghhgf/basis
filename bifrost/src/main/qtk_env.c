#include "os/qtk_spinlock.h"
#include "os/qtk_alloc.h"
#include "os/qtk_base.h"
#include "qtk_env.h"

#include <assert.h>
#include <string.h>

struct bifrost_env {
	qtk_spinlock_t lock;
	lua_State *L;
};

static struct bifrost_env *E = NULL;

static const char * load_config = "\
    local function getenv(name) return assert(os.getenv(name), [[os.getenv() failed: ]] .. name) end\n\
    local sep = package.config:sub(1,1)\n\
    local current_path = [[.]]..sep\n\
    local function include(filename)\n\
        local last_path = current_path\n\
        local path, name = filename:match([[(.*]]..sep..[[)(.*)$]])\n\
        if path then\n\
            if path:sub(1,1) == sep then	-- root\n\
                current_path = path\n\
            else\n\
                current_path = current_path .. path\n\
            end\n\
        else\n\
            name = filename\n\
        end\n\
        local f = assert(io.open(current_path .. name))\n\
        local code = assert(f:read [[a]])\n\
        code = string.gsub(code, [[%$([%w_%d]+)]], getenv)\n\
        f:close()\n\
        assert(load(code,[[@]]..filename,[[t]]))()\n\
        current_path = last_path\n\
    end\n\
    local config_name = ...\n\
    include(config_name)\n\
";

static const char *qtk_getenv_str(const char *key)
{
	QTK_SPIN_LOCK(E)

	lua_State *L = E->L;
	lua_getglobal(L, key);
	const char *result = lua_tostring(L, -1);
	lua_pop(L, 1);

	QTK_SPIN_UNLOCK(E);

	return result;
}

static void qtk_setenv_str(const char *key, const char *value)
{
	QTK_SPIN_LOCK(E)

	lua_State *L = E->L;
	lua_getglobal(L, key);
	assert(lua_isnil(L, -1));
	lua_pop(L,1);
	lua_pushstring(L,value);
	lua_setglobal(L,key);

	QTK_SPIN_UNLOCK(E);
}

static lua_Number qtk_getenv_num(const char *key, int *succ)
{
    QTK_SPIN_LOCK(E);

	lua_State *L = E->L;
	lua_getglobal(L, key);
    lua_Number result = lua_tonumberx(L, -1, succ);
    lua_pop(L, 1);

	QTK_SPIN_UNLOCK(E);
    return result;
}

static void qtk_setenv_num(const char *key, lua_Number value)
{
	QTK_SPIN_LOCK(E)

	lua_State *L = E->L;
	lua_getglobal(L, key);
	assert(lua_isnil(L, -1));
	lua_pop(L,1);
	lua_pushnumber(L,value);
	lua_setglobal(L,key);

	QTK_SPIN_UNLOCK(E);
}

static int qtk_getenv_bool(const char *key, int *succ)
{
    QTK_SPIN_LOCK(E);

	lua_State *L = E->L;
	lua_getglobal(L, key);
    *succ = lua_isboolean(L, -1);
    int result = lua_toboolean(L, -1);
    lua_pop(L, 1);

	QTK_SPIN_UNLOCK(E);
    return result;
}

static void qtk_setenv_bool(const char *key, int value)
{
	QTK_SPIN_LOCK(E)

	lua_State *L = E->L;
	lua_getglobal(L, key);
	assert(lua_isnil(L, -1));
	lua_pop(L,1);
	lua_pushboolean(L,value);
	lua_setglobal(L,key);

	QTK_SPIN_UNLOCK(E);
}

const char *qtk_optstring(const char *key, const char *opt)
{
    const char *str = qtk_getenv_str(key);
    if (NULL == str) {
        if (opt) {
            qtk_setenv_str(key, opt);
            opt = qtk_getenv_str(key);
        }
        return opt;
    }
    return str;
}

lua_Number qtk_optnumber(const char *key, lua_Number opt)
{
    int succ;
    lua_Number ret = qtk_getenv_num(key, &succ);
    if (succ) { return ret; }
    qtk_setenv_num(key, opt);
    ret = qtk_getenv_num(key, &succ);
    assert(succ);
    return ret;
}

int qtk_optboolean(const char *key, int opt)
{
    int succ;
    int ret = qtk_getenv_bool(key, &succ);
    if (succ) { return ret; }
    qtk_setenv_bool(key, opt);
    ret = qtk_getenv_bool(key, &succ);
    assert(succ);
    return ret;
}

void qtk_env_init()
{
	E = qtk_malloc(sizeof(*E));
	QTK_SPIN_INIT(E)
	E->L = luaL_newstate();
    luaL_openlibs(E->L);
}

int qtk_env_update(const char *cfn, int len)
{
    lua_State *L = E->L;
    int ret = luaL_loadbufferx(L, load_config, strlen(load_config), "CONFIG", "t");
    assert(ret == LUA_OK);
    lua_pushlstring(L, cfn, len);
    ret = lua_pcall(L, 1, 1, 0);
    if (ret != LUA_OK) {
        qtk_debug("update config fail: %s\n", lua_tostring(L, -1));
        return -1;
    }
    return 0;
}

void qtk_env_clean()
{
    assert(E);
    lua_close(E->L);
    qtk_free(E);
}
