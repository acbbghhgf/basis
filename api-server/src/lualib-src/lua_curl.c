#include "third/lua5.3.4/src/lua.h"
#include "third/lua5.3.4/src/lauxlib.h"

#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

#include "base/qtk_strbuf.h"
#include "wtk/core/wtk_strbuf.h"

#define LUA_CURLHANDLE  "CURL *"

#define tolcurl(L, idx) *((CURL **)luaL_checkudata(L, idx, LUA_CURLHANDLE))

static size_t _recv_cb(char *ptr, size_t size, size_t nmemb, void *ud)
{
    qtk_strbuf_t *buf = (qtk_strbuf_t *)ud;
    size_t len = size * nmemb;
    if (len > 0) {
        qtk_strbuf_append(buf, ptr, len);
    }
    return len;
}

static int leasy_init(lua_State *L)
{
    CURL *curl = curl_easy_init();
    if (NULL == curl) {
        return 0;
    }
    CURL **curl_lua = lua_newuserdata(L, sizeof(curl));
    *curl_lua = curl;
    luaL_setmetatable(L, LUA_CURLHANDLE);
    return 1;
}

static int leasy_cleanup(lua_State *L)
{
    CURL *c = tolcurl(L, 1);
    if (c) {
        curl_easy_cleanup(c);
    }
    return 0;
}

/* @param 1 is CURL*
 * @param 2 is url
 * @param 3 is postfield
 * @param 4 is a tabel(http request hdr), may be nil
 * @return value is status(number), body(string), hdr(table)
 * TODO Support Get Response Hdr
 * */
static int lpost(lua_State *L)
{
    char tmp[512];
    CURL *c = tolcurl(L, 1);
    const char *url = luaL_checkstring(L, 2);
    if (NULL == url) return 0;
    curl_easy_setopt(c, CURLOPT_URL, url);
    struct curl_slist *hdr = NULL;
    int ret = 0;
    if (LUA_TSTRING == lua_type(L, 3)) {
        size_t len;
        const char *field = lua_tolstring(L, 3, &len);
        curl_easy_setopt(c, CURLOPT_POSTFIELDS, field);
        snprintf(tmp, sizeof(tmp), "content-length:%ld", len);
        hdr = curl_slist_append(hdr, tmp);
        if (hdr == NULL) return 0;
    }
    if (LUA_TTABLE == lua_type(L, 4)) {
        lua_settop(L, 4);
        lua_pushnil(L);
        while (lua_next(L, -2)) {
            const char *key = lua_tostring(L, -2);
            const char *value = lua_tostring(L, -1);
            if (key && value) {
                snprintf(tmp, sizeof(tmp), "%s:%s", key, value);
                hdr = curl_slist_append(hdr, tmp);
                if (hdr == NULL) return 0;
            }
            lua_pop(L, 1);
        }
    }
    qtk_strbuf_t *buf = qtk_strbuf_new(128, -1);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, buf);
    curl_easy_setopt(c, CURLOPT_POST, 1);
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, _recv_cb);
    if (hdr) curl_easy_setopt(c, CURLOPT_HTTPHEADER, hdr);
    if (curl_easy_perform(c) != CURLE_OK) {
        goto end;
    }
    long status;
    if (curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &status) != CURLE_OK) {
        goto end;
    }
    lua_pushinteger(L, status);
    lua_pushlstring(L, buf->data, buf->len);
    ret = 2;
end:
    if (buf) qtk_strbuf_delete(buf);
    if (hdr) curl_slist_free_all(hdr);
    return ret;
}

static int lreset(lua_State *L)
{
    CURL *c = tolcurl(L, 1);
    curl_easy_reset(c);
    return 0;
}

static const luaL_Reg lib[] = {
    {"post", lpost},
    {"reset", lreset},
    {"__gc", leasy_cleanup},
    {NULL, NULL},
};

static void createmeta(lua_State *L)
{
    luaL_newmetatable(L, LUA_CURLHANDLE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, lib, 0);
    lua_pop(L, 1);
}

static int lcurl_exit(lua_State *L)
{
    curl_global_cleanup();
    return 0;
}

LUAMOD_API int luaopen_curl(lua_State *L)
{
	luaL_checkversion(L);
    luaL_Reg l[] = {
        {"new", leasy_init},
        {NULL, NULL},
    };
    luaL_newlib(L, l);
    createmeta(L);
    curl_global_init(CURL_GLOBAL_NOTHING);
    luaL_newlib(L, l);
    lua_newtable(L);
    lua_pushcfunction(L, lcurl_exit);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);
    return 1;
}
