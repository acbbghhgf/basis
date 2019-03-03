#include <assert.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <unistd.h>
#include "qtk/core/qtk_deque.h"
#include "qtk/proto/http/qtk_zhttp_parser.h"


static void push_headers(lua_State *L, wtk_strbuf_t *buf) {
    lua_pushnil(L);
    while (lua_next(L, -2)) {
        size_t nkey, nval;
        char tmp[16];
        const char *val = NULL;
        const char *key = luaL_checklstring(L, -2, &nkey);
        if (key) {
            int type = lua_type(L, -1);
            if (type == LUA_TSTRING) {
                val = luaL_checklstring(L, -1, &nval);
            } else if (type == LUA_TNUMBER) {
                int v = lua_tointeger(L, -1);
                nval = snprintf(tmp, sizeof(tmp), "%d", v);
                val = tmp;
            }
        }
        if (val) {
            wtk_strbuf_push(buf, key, nkey);
            wtk_strbuf_push_c(buf, ':');
            wtk_strbuf_push(buf, val, nval);
            wtk_strbuf_push_s(buf, "\r\n");
        }
        lua_pop(L, 1);
    }
}


static int lrequest(lua_State *L) {
    size_t nmeth, nurl;
    const char *pmeth = luaL_checklstring(L, 1, &nmeth);
    const char *purl = luaL_checklstring(L, 2, &nurl);
    if (NULL == pmeth || NULL == purl) return 0;
    wtk_strbuf_t *buf = wtk_strbuf_new(1024, 1);
    size_t n;
    const char *p = luaL_checklstring(L, 3, &n);
    char tmp[128];
    wtk_strbuf_push(buf, pmeth, nmeth);
    wtk_strbuf_push_c(buf, ' ');
    wtk_strbuf_push(buf, purl, nurl);
    wtk_strbuf_push_s(buf, " HTTP/1.1\r\n");
    if (lua_gettop(L) == 4 && lua_type(L, 4) == LUA_TTABLE) {
        push_headers(L, buf);
    }
    wtk_strbuf_push(buf, tmp, snprintf(tmp, sizeof(tmp), "Content-Length:%lu\r\n\r\n", n));
    wtk_strbuf_push(buf, p, n);
    lua_pushlstring(L, buf->data, buf->pos);
    wtk_strbuf_delete(buf);
    return 1;
}


/*
  @params: status_code integer
           body string
           headers table
 */
static int lresponse(lua_State *L) {
    int code = lua_tointeger(L, 1);
    size_t lbody;
    const char *pbody = luaL_checklstring(L, 2, &lbody);
    wtk_strbuf_t *buf = wtk_strbuf_new(1024, 1);
    char tmp[128];
    wtk_strbuf_push(buf, tmp, snprintf(tmp, sizeof(tmp), "HTTP/1.1 %d %s\r\n",
                                       code, qtk_zhttp_status_str(code)));
    if (lua_gettop(L) == 3 && lua_type(L, 3) == LUA_TTABLE) {
        push_headers(L, buf);
    }
    wtk_strbuf_push(buf, tmp, snprintf(tmp, sizeof(tmp), "Content-Length:%lu\r\n\r\n", lbody));
    wtk_strbuf_push(buf, pbody, lbody);
    lua_pushlstring(L, buf->data, buf->pos);
    wtk_strbuf_delete(buf);
    return 1;
}


static int lhttpparser(lua_State *L) {
    qtk_zhttp_parser_t *parser = lua_newuserdata(L, sizeof(*parser));
    qtk_zhttp_parser_init(parser);
    assert(luaL_getmetatable(L, "httpparser"));
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


static int lhttpparser_gethdr(lua_State *L) {
    qtk_zhttp_parser_t *parser = lua_touserdata(L, 1);
    wtk_string_t *value = NULL;
    size_t klen = 0;
    const char *key = luaL_checklstring(L, 2, &klen);
    if (key) {
        value = qtk_httphdr_find_hdr(parser->pack, key, klen);
    }
    if (value) {
        lua_pushlstring(L, value->data, value->len);
        return 1;
    }
    return 0;
}


static int lhttpparser_gc(lua_State *L) {
    assert(1 == lua_gettop(L));
    qtk_zhttp_parser_t *parser = lua_touserdata(L, 1);
    qtk_zhttp_parser_clean(parser);
    return 0;
}


typedef struct Num_Reg {
    const char *name;
    int val;
} Num_Reg_t;


LUAMOD_API int luaopen_zeus_http(lua_State *L) {
    int i;
    luaL_checkversion(L);
    assert(luaL_getmetatable(L, "buffer"));
    lua_pop(L, 1);
    assert(0 == luaL_getmetatable(L, "httpparser"));
    luaL_Reg httpparser_meta[] = {
        {"feed", lhttpparser_feed},
        {"getheader", lhttpparser_gethdr},
        {"__gc", lhttpparser_gc},
        {NULL, NULL},
    };
    luaL_newmetatable(L, "httpparser");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, httpparser_meta, 0);

    luaL_Reg iris[] = {
        {"parser", lhttpparser},
        {"request", lrequest},
        {"response", lresponse},
        {NULL, NULL},
    };
    luaL_newlib(L, iris);

    Num_Reg_t http_const[] = {
        {"GET", HTTP_GET},
        {"POST", HTTP_POST},
        {"PUT", HTTP_PUT},
        {"HEAD", HTTP_HEAD},
        {"DELETE", HTTP_DELETE},
        {NULL, 0},
    };
    for (i = 0; i < sizeof(http_const)/sizeof(http_const[0])-1; ++i) {
        lua_pushinteger(L, http_const[i].val);
        lua_setfield(L, -2, http_const[i].name);
    }
    return 1;
}
