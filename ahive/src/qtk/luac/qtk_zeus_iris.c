#include <assert.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <unistd.h>
#include "qtk/core/qtk_deque.h"
#include "qtk/proto/http/qtk_zhttp_parser.h"


static int lhttppack(lua_State *L) {
    size_t nmeth, nurl;
    const char *pmeth = luaL_checklstring(L, 1, &nmeth);
    const char *purl = luaL_checklstring(L, 2, &nurl);
    int type = lua_type(L, 3);
    if (NULL == pmeth || NULL == purl) return 0;
    if (type == LUA_TUSERDATA) {
        qtk_deque_t *body = lua_touserdata(L, 3);
        qtk_deque_t *pack = lua_newuserdata(L, sizeof(*pack));
        char tmp[128];
        int n;
        assert(luaL_getmetatable(L, "buffer"));
        lua_setmetatable(L, -2);
        qtk_deque_clone(pack, body);
        n = snprintf(tmp, sizeof(tmp), " HTTP/1.1\r\nContent-Length:%d\r\n\r\n", pack->len);
        qtk_deque_push_front(pack, tmp, n);
        qtk_deque_push_front(pack, purl, nurl);
        qtk_deque_push_front(pack, " ", 1);
        qtk_deque_push_front(pack, pmeth, nmeth);
        return 1;
    } else {
        wtk_strbuf_t *buf = wtk_strbuf_new(1024, 1);
        size_t n;
        const char *p = luaL_checklstring(L, 3, &n);
        char tmp[32];
        wtk_strbuf_push(buf, pmeth, nmeth);
        wtk_strbuf_push_c(buf, ' ');
        wtk_strbuf_push(buf, purl, nurl);
        wtk_strbuf_push_s(buf, " HTTP/1.1\r\nContent-Length:");
        wtk_strbuf_push(buf, tmp, snprintf(tmp, sizeof(tmp), "%lu\r\n\r\n", n));
        wtk_strbuf_push(buf, p, n);
        lua_pushlstring(L, buf->data, buf->pos);
        wtk_strbuf_delete(buf);
        return 1;
    }
    return 0;
}


static int lhttpparser(lua_State *L) {
    qtk_zhttp_parser_t *parser = lua_newuserdata(L, sizeof(*parser));
    qtk_zhttp_parser_init(parser);
    assert(luaL_getmetatable(L, "iris_httpparser"));
    lua_setmetatable(L, -2);
    return 1;
}


/*
  input: parser, buffer
  output: ret[,topic or status, body]
          pack raise: ret is type(1:request, 2:response)
          no pack: ret is 0
          error occur: ret is negative
 */
static int lhttpparser_feed(lua_State *L) {
    qtk_zhttp_parser_t *parser = lua_touserdata(L, 1);
    qtk_deque_t *dq = lua_touserdata(L, 2);
    int ret = parser->handler(parser, dq);
    if (ret > 0) {
        qtk_http_hdr_t *hdr = parser->pack;
        qtk_deque_t *pack = NULL;
        int type = parser->type;
        lua_pushinteger(L, type);
        if (type == HTTP_REQUEST_PARSER) {
            lua_pushlstring(L, hdr->req.url.data, hdr->req.url.len);
        } else {
            lua_pushinteger(L, (int)hdr->rep.status);
        }
        pack = lua_newuserdata(L, sizeof(*pack));
        qtk_deque_init(pack, 1024, 1024*1024, 1);
        luaL_getmetatable(L, "buffer");
        lua_setmetatable(L, -2);
        qtk_zhttp_parser_get_body(parser, dq, pack);
        if (type == HTTP_REQUEST_PARSER) {
            lua_pushinteger(L, hdr->req.method);
            return 4;
        } else {
            return 3;
        }
    } else {
        lua_pushinteger(L, ret);
        return 1;
    }
}


static int lhttpparser_gc(lua_State *L) {
    assert(1 == lua_gettop(L));
    qtk_zhttp_parser_t *parser = lua_touserdata(L, 1);
    qtk_zhttp_parser_clean(parser);
    return 0;
}


LUAMOD_API int luaopen_zeus_iris(lua_State *L) {
    luaL_checkversion(L);
    assert(luaL_getmetatable(L, "buffer"));
    lua_pop(L, 1);
    assert(0 == luaL_getmetatable(L, "iris_httpparser"));
    luaL_Reg httpparser_meta[] = {
        {"feed", lhttpparser_feed},
        {"__gc", lhttpparser_gc},
        {NULL, NULL},
    };
    luaL_newmetatable(L, "iris_httpparser");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, httpparser_meta, 0);

    luaL_Reg iris[] = {
        {"httpparser", lhttpparser},
        {"httppack", lhttppack},
        {NULL, NULL},
    };
    luaL_newlib(L, iris);
    return 1;
}
