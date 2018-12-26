#include "third/lua5.3.4/src/lua.h"
#include "third/lua5.3.4/src/lualib.h"
#include "third/lua5.3.4/src/lauxlib.h"

#include "os/qtk_alloc.h"

#include <assert.h>

#define LUA_LISTHANDLE    "LIST*"

struct NODE {
    struct NODE *next;
    struct NODE *prev;
    int obj_ref;
};

struct LIST {
    struct NODE *head;
    struct NODE *tail;
    int length;
};

#define tollist(L, idx)      ((struct LIST *)luaL_checkudata(L, idx, LUA_LISTHANDLE))

/* element is already at top of stack */
static struct NODE *new_node(lua_State *L)
{
    struct NODE *node = qtk_calloc(1, sizeof(*node));
    int ret = lua_getmetatable(L, -1);
    if (0 == ret) {                     /* element has no metatable */
        lua_newtable(L);
        lua_pushlightuserdata(L, node);
        lua_setfield(L, -2, "node");
        lua_setmetatable(L, -2);
    } else {                            /* update element's metatable */
        lua_pushlightuserdata(L, node);
        lua_setfield(L, -2, "node");
        lua_pop(L, 1);
    }
    node->obj_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return node;
}

static struct NODE *get_node(lua_State *L, int idx)
{
    assert(lua_getmetatable(L, idx));
    lua_getfield(L, -1, "node");
    struct NODE *ret = lua_isuserdata(L, -1) ? lua_touserdata(L, -1) : NULL;
    lua_pop(L, 1);
    return ret;
}

static void unlink_node(struct LIST *l, struct NODE *node)
{
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
    if (l->head == node) l->head = node->next;
    if (l->tail == node) l->tail = node->prev;
}

static void head_link(struct LIST *l, struct NODE *node)
{
    node->prev = NULL;
    node->next = l->head;
    if (l->head) {
        l->head->prev = node;
    } else {
        assert(l->tail == NULL);
        l->tail = node;
    }
    l->head = node;
}

static void tail_link(struct LIST *l, struct NODE *node)
{
    node->next = NULL;
    node->prev = l->tail;
    if (l->tail) {
        l->tail->next = node;
    } else {
        assert(l->head == NULL);
        l->head = node;
    }
    l->tail = node;
}

static void remove_node(lua_State *L, struct LIST *l, struct NODE *node)
{
    if (NULL == node) return ;
    unlink_node(l, node);
    luaL_unref(L, LUA_REGISTRYINDEX, node->obj_ref);
    qtk_free(node);
    l->length --;
}

static int lhead_insert(lua_State *L)
{
    struct LIST *l = tollist(L, 1);
    luaL_checkany(L, 2);
    lua_settop(L, 2);
    struct NODE *new = new_node(L);
    head_link(l, new);
    l->length ++;
    return 0;
}

static int ltail_insert(lua_State *L)
{
    struct LIST *l = tollist(L, 1);
    luaL_checkany(L, 2);
    lua_settop(L, 2);
    struct NODE *new = new_node(L);
    tail_link(l, new);
    l->length ++;
    return 0;
}

static int ltouch_head(lua_State *L)
{
    struct LIST *l = tollist(L, 1);
    if (NULL == l->head) return 0;
    lua_rawgeti(L, LUA_REGISTRYINDEX, l->head->obj_ref);
    return 1;
}

static int ltouch_tail(lua_State *L)
{
    struct LIST *l = tollist(L, 1);
    if (NULL == l->tail) return 0;
    lua_rawgeti(L, LUA_REGISTRYINDEX, l->tail->obj_ref);
    return 1;
}

static int lget_head(lua_State *L)
{
    struct LIST *l = tollist(L, 1);
    if (NULL == l->head) return 0;
    lua_rawgeti(L, LUA_REGISTRYINDEX, l->head->obj_ref);
    remove_node(L, l, l->head);
    return 1;
}

static int lget_tail(lua_State *L)
{
    struct LIST *l = tollist(L, 1);
    if (NULL == l->tail) return 0;
    lua_rawgeti(L, LUA_REGISTRYINDEX, l->tail->obj_ref);
    remove_node(L, l, l->tail);
    return 1;
}

static int lmove_tail(lua_State *L)
{
    struct LIST *l = tollist(L, 1);
    luaL_checkany(L, 2);
    struct NODE *node = get_node(L, 2);
    assert(node);
    unlink_node(l, node);
    tail_link(l, node);
    return 0;
}

static int lremove(lua_State *L)
{
    struct LIST *l = tollist(L, 1);
    luaL_checkany(L, 2);
    struct NODE *node = get_node(L, 2);
    assert(node);
    remove_node(L, l, node);
    return 0;
}

static int llength(lua_State *L)
{
    struct LIST *l = tollist(L, 1);
    lua_pushinteger(L, l->length);
    return 1;
}

static int lgc(lua_State *L)
{
    struct LIST *l = tollist(L, 1);
    while (l->head) remove_node(L, l, l->head);
    return 0;
}

static int ltostring(lua_State *L)
{
    struct LIST *l = tollist(L, 1);
    lua_pushfstring(L, "list @%p,length=%d", l, l->length);
    return 1;
}

static const luaL_Reg listlib[] = {
    {"headinsert", lhead_insert},
    {"tailinsert", ltail_insert},
    {"touchhead", ltouch_head},
    {"touchtail", ltouch_tail},
    {"gethead", lget_head},
    {"gettail", lget_tail},
    {"movetail", lmove_tail},
    {"remove", lremove},
    {"length", llength},
    {"__gc", lgc},
    {"__tostring", ltostring},
    {NULL, NULL}
};

static void createmeta(lua_State *L)
{
    luaL_newmetatable(L, LUA_LISTHANDLE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, listlib, 0);
    lua_pop(L, 1);
}

static int lnew(lua_State *L)
{
    struct LIST *l = lua_newuserdata(L, sizeof(*l));
    l->head = l->tail = NULL;
    l->length = 0;
    luaL_setmetatable(L, LUA_LISTHANDLE);
    return 1;
}

static const luaL_Reg mlib[] = {
    {"new", lnew},
    {NULL,  NULL}
};

LUAMOD_API int luaopen_list(lua_State *L)
{
    luaL_newlib(L, mlib);
    createmeta(L);
    return 1;
}
