#include <assert.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <unistd.h>
#include "qtk/core/qtk_deque.h"


static int lopen(lua_State *L) {
    size_t sz;
    const char *p = luaL_checklstring(L, 1, &sz);
    char tmp[sz+1];
    strcpy(tmp, p);
    int fd = open(tmp, O_RDONLY);
    if (fd == INVALID_FD) {
        return 0;
    } else {
        lua_pushinteger(L, fd);
        return 1;
    }
}


static int lread(lua_State *L) {
    int fd = lua_tointeger(L, 1);
    qtk_deque_t *dq = lua_touserdata(L, 2);
    char buf[1024];
    int readed = 0;
    while ((readed = read(fd, buf, sizeof(buf))) > 0) {
        qtk_deque_push(dq, buf, readed);
    }
    if (readed) {
        lua_pushboolean(L, 0);
    } else {
        lua_pushboolean(L, 1);
    }
    return 1;
}


static int lreadstring(lua_State *L) {
    int fd = lua_tointeger(L, 1);
    qtk_deque_t *dq = qtk_deque_new(1024, 1024*1024, 1);
    char buf[1024];
    int readed = 0;
    int nret = 0;
    while ((readed = read(fd, buf, sizeof(buf))) > 0) {
        qtk_deque_push(dq, buf, readed);
    }
    if (!readed) {
        wtk_string_t *tmp = wtk_string_new(dq->len);
        qtk_deque_pop(dq, tmp->data, dq->len);
        lua_pushlstring(L, tmp->data, tmp->len);
        wtk_string_delete(tmp);
        nret = 1;
    }
    qtk_deque_delete(dq);
    return nret;
}


static int lclose(lua_State *L) {
    int fd = lua_tointeger(L, 1);
    close(fd);
    return 0;
}


LUAMOD_API int luaopen_zeus_reader(lua_State *L) {
    luaL_checkversion(L);
    luaL_Reg reader[] = {
        {"open", lopen},
        {"read", lread},
        {"readstring", lreadstring},
        {"close", lclose},
        {NULL, NULL},
    };
    luaL_newlib(L, reader);
    return 1;
}
