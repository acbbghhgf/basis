#include "qtk/sframe/qtk_sframe.h"
#include "qtk/websocket/qtk_ws_protobuf.h"
#include "wtk/os/wtk_thread.h"
#include <assert.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"


typedef struct test_demo test_demo_t;

struct test_demo {
    wtk_thread_t thread;
    void *frame;
    unsigned run:1;
};


static int ldecode_var(lua_State *L, qpb_variant_t *v, int desctype) {
    switch (desctype) {
    case QPB_DESCTYPE_BOOL:
        lua_pushboolean(L, qpb_var_to_bool(v));
        break;
    case QPB_DESCTYPE_BYTES:
    case QPB_DESCTYPE_STRING:
        lua_pushlstring(L, v->v.s->data, v->v.s->len);
        break;
    case QPB_DESCTYPE_DOUBLE:
        lua_pushnumber(L, qpb_var_to_double(v));
        break;
    case QPB_DESCTYPE_FLOAT:
        lua_pushnumber(L, qpb_var_to_float(v));
        break;
    case QPB_DESCTYPE_ENUM:
        lua_pushinteger(L, qpb_var_to_enum(v));
        break;
    case QPB_DESCTYPE_FIXED32:
        lua_pushinteger(L, qpb_var_to_fixed32(v));
        break;
    case QPB_DESCTYPE_FIXED64:
        lua_pushinteger(L, qpb_var_to_fixed64(v));
        break;
    case QPB_DESCTYPE_INT32:
        lua_pushinteger(L, qpb_var_to_int32(v));
        break;
    case QPB_DESCTYPE_INT64:
        lua_pushinteger(L, qpb_var_to_int64(v));
        break;
    case QPB_DESCTYPE_SFIXED64:
        lua_pushinteger(L, qpb_var_to_sfixed64(v));
        break;
    case QPB_DESCTYPE_SFIXED32:
        lua_pushinteger(L, qpb_var_to_sfixed32(v));
        break;
    case QPB_DESCTYPE_SINT64:
        lua_pushinteger(L, qpb_var_to_sint64(v));
        break;
    case QPB_DESCTYPE_SINT32:
        lua_pushinteger(L, qpb_var_to_sint32(v));
        break;
    case QPB_DESCTYPE_UINT64:
        lua_pushinteger(L, qpb_var_to_uint64(v));
        break;
    case QPB_DESCTYPE_UINT32:
        lua_pushinteger(L, qpb_var_to_uint32(v));
        break;
    default:
        assert(0);
    }
    return 1;
}


static int ldecode_msg(lua_State *L, qtk_protobuf_t *pb, qpb_variant_t *v) {
    if (QPB_CTYPE_DELIMITED == v->type) {
        qtk_pb_decode_message(pb, v);
    }
    int tIdx = lua_gettop(L);
    lua_newtable(L);
    int rIdx = lua_gettop(L);
    lua_pushnil(L);
    int kIdx= lua_gettop(L);
    while (0 != lua_next(L, tIdx)) {
        int aIdx = lua_gettop(L);
        lua_rawgeti(L, aIdx, 1);
        int id = lua_tointeger(L, -1);
        qpb_variant_t *vi = qpb_var_array_get(v, id);
        lua_rawgeti(L, aIdx, 3);
        int label = lua_tointeger(L, -1);
        if (NULL == vi || 0 == vi->type) {
            if (QPB_LABEL_REQUIRED == label) {
                wtk_debug("%s is required\r\n", lua_tostring(L, kIdx));
                goto err;
            } else {
                goto skip;
            }
        }
        lua_rawgeti(L, aIdx, 2);
        int ttype = lua_type(L, -1);
        int typIdx = lua_gettop(L);
        int desctype = lua_tointeger(L, -1);
        if (QPB_LABEL_REPEATED == label) {
            if (vi->type == QPB_CTYPE_DELIMITED) {
                qpb_field_t field;
                field.label = QPB_LABEL_OPTIONAL;
                field.desctype = desctype;
                qtk_pb_decode_array(pb, vi, &field);
            }
            assert(vi->type == QPB_CTYPE_ARRAY);
            qpb_variant_t *s = vi->v.a->data;
            qpb_variant_t *e = s + vi->v.a->len;
            lua_pushvalue(L, kIdx);
            lua_newtable(L);
            int arrIdx = lua_gettop(L);
            int i;
            for (i = 1; s < e && s->type; s++, i++) {
                if (ttype == LUA_TTABLE) {
                    lua_pushvalue(L, typIdx);
                    ldecode_msg(L, pb, s);
                    lua_rawseti(L, arrIdx, i);
                    lua_settop(L, arrIdx);
                } else {
                    assert(ttype == LUA_TNUMBER);
                    ldecode_var(L, s, desctype);
                    lua_rawseti(L, arrIdx, i);
                }
            }
        } else {
            if (ttype == LUA_TTABLE) {
                lua_pushvalue(L, typIdx);
                ldecode_msg(L, pb, vi);
                lua_pushvalue(L, kIdx);
                lua_pushvalue(L, -2);
            } else {
                assert(ttype == LUA_TNUMBER);
                lua_pushvalue(L, kIdx);
                ldecode_var(L, vi, desctype);
            }
        }
        lua_rawset(L, rIdx);
    skip:
        lua_settop(L, kIdx);
    }
    lua_settop(L, rIdx);
    return 1;
err:
    return 0;
}


static int ldecode(lua_State *L) {
    qpb_variant_t *v = lua_touserdata(L, -2);
    qtk_protobuf_t pb;
    qtk_protobuf_init2(&pb, NULL);
    int ret = ldecode_msg(L, &pb, v);
    qtk_protobuf_clean(&pb);
    return ret;
}


static int lencode_var(lua_State *L, qpb_variant_t *v, int desctype) {
    switch (desctype) {
    case QPB_DESCTYPE_BOOL:
        qpb_var_set_bool(v, lua_tointeger(L, -1));
        break;
    case QPB_DESCTYPE_BYTES:
    case QPB_DESCTYPE_STRING: {
        size_t len = 0;
        const char *p = lua_tolstring(L, -1, &len);
        if (p) {
            qpb_var_set_string2(v, p, len);
        }
        break;
    }
    case QPB_DESCTYPE_DOUBLE:
        qpb_var_set_double(v, lua_tonumber(L, -1));
        break;
    case QPB_DESCTYPE_FLOAT:
        qpb_var_set_float(v, lua_tonumber(L, -1));
        break;
    case QPB_DESCTYPE_ENUM:
        qpb_var_set_enum(v, lua_tointeger(L, -1));
        break;
    case QPB_DESCTYPE_FIXED32:
        qpb_var_set_fixed32(v, lua_tointeger(L, -1));
        break;
    case QPB_DESCTYPE_FIXED64:
        qpb_var_set_fixed64(v, lua_tointeger(L, -1));
        break;
    case QPB_DESCTYPE_INT32:
        qpb_var_set_int32(v, lua_tointeger(L, -1));
        break;
    case QPB_DESCTYPE_INT64:
        qpb_var_set_int64(v, lua_tointeger(L, -1));
        break;
    case QPB_DESCTYPE_SFIXED64:
        qpb_var_set_sfixed64(v, lua_tointeger(L, -1));
        break;
    case QPB_DESCTYPE_SFIXED32:
        qpb_var_set_sfixed32(v, lua_tointeger(L, -1));
        break;
    case QPB_DESCTYPE_SINT64:
        qpb_var_set_sint64(v, lua_tointeger(L, -1));
        break;
    case QPB_DESCTYPE_SINT32:
        qpb_var_set_sint32(v, lua_tointeger(L, -1));
        break;
    case QPB_DESCTYPE_UINT64:
        qpb_var_set_uint64(v, lua_tointeger(L, -1));
        break;
    case QPB_DESCTYPE_UINT32:
        qpb_var_set_uint32(v, lua_tointeger(L, -1));
        break;
    default:
        assert(0);
    }
    return 0;
}


static int lencode_msg(lua_State *L) {
    qpb_variant_t *v = lua_touserdata(L, -1);
    if (0 == v->type) {
        v->v.a = qtk_array_new2(sizeof(qpb_variant_t), 8);
        v->type = QPB_CTYPE_MESSAGE;
    }
    int tIdx = lua_gettop(L) -1;
    int dIdx = tIdx - 1;
    lua_pushnil(L);
    int kIdx= lua_gettop(L);
    while (0 != lua_next(L, tIdx)) {
        int aIdx = lua_gettop(L);
        lua_rawgeti(L, aIdx, 1);
        int id = lua_tointeger(L, -1);
        lua_rawgeti(L, aIdx, 3);
        int label = lua_tointeger(L, -1);
        lua_pushvalue(L, kIdx);
        int vtype = lua_rawget(L, dIdx);
        if (LUA_TNIL == vtype) {
            if (QPB_LABEL_REQUIRED == label) {
                wtk_debug("%s is required\r\n", lua_tostring(L, kIdx));
                goto err;
            } else {
                goto skip;
            }
        }
        int curIdx = lua_gettop(L);
        lua_rawgeti(L, aIdx, 2);
        int ttype = lua_type(L, -1);
        int typIdx = lua_gettop(L);
        int desctype = lua_tointeger(L, -1);
        if (QPB_LABEL_REPEATED == label) {
            int i = 1;
            qpb_variant_t new;
            qpb_variant_init(&new);
            qpb_var_set_array(&new, 8);
            if (ttype == LUA_TTABLE || desctype == QPB_DESCTYPE_STRING || desctype == QPB_DESCTYPE_GROUP
                || desctype == QPB_DESCTYPE_BYTES || desctype == QPB_DESCTYPE_MESSAGE) {
                /* without pack */
                while (LUA_TNIL != lua_rawgeti(L, curIdx, i++)) {
                    qpb_variant_t *e = qtk_array_push(new.v.a);
                    if (desctype == QPB_DESCTYPE_STRING || desctype == QPB_DESCTYPE_BYTES) {
                        size_t len = 0;
                        const char *p = lua_tolstring(L, -1, &len);
                        e->v.s = wtk_string_new(len);
                        memcpy(e->v.s->data, p, len);
                        e->type = QPB_CTYPE_STRING;
                    } else {
                        lua_pushvalue(L, typIdx);
                        lua_pushlightuserdata(L, e);
                        lencode_msg(L);
                    }
                    lua_settop(L, typIdx);
                }
            } else {
                /* with pack */
                while (LUA_TNIL != lua_rawgeti(L, curIdx, i++)) {
                    qpb_variant_t *e = qtk_array_push(new.v.a);
                    lencode_var(L, e, desctype);
                    lua_settop(L, typIdx);
                }
                new.type = QPB_CTYPE_PACK;
            }
            qtk_array_set(v->v.a, id, &new);
        } else if (LUA_TNIL != vtype) {
            qpb_variant_t new;
            qpb_variant_init(&new);
            lua_pushvalue(L, curIdx);
            if (LUA_TTABLE == ttype) {
                lua_pushvalue(L, typIdx);
                lua_pushlightuserdata(L, &new);
                lencode_msg(L);
            } else {
                lencode_var(L, &new, desctype);
            }
            qtk_array_set(v->v.a, id, &new);
        }
    skip:
        lua_settop(L, kIdx);
    }
    return 0;
err:
    return -1;
}


static int lencode(lua_State *L) {
    /* this is not safe, Be careful of memory leak */
    qpb_variant_t *v = qpb_variant_new();
    lua_pushlightuserdata(L, v);
    lencode_msg(L);
    lua_pushlightuserdata(L, v);
    return 1;
}


static int demo_route(void *data, wtk_thread_t *t) {
    test_demo_t *demo = data;
    void *frame = demo->frame;
    qtk_sframe_method_t *method = frame;

    lua_State *L = luaL_newstate();
    /* close code cache for valgrind leak check */
    luaopen_cache(L);
    lua_pushstring(L, "mode");
    lua_gettable(L, -2);
    lua_pushstring(L, "OFF");
    int r = lua_pcall(L, 1, 0, 1);
    if (LUA_OK != r) {
        wtk_debug("disable cache failed\r\n");
    }
    luaopen_base(L);
    const char *path = "./lua/?.lua;";
    lua_pushstring(L, path);
    const char *cpath = "./lua/?.so;";
    lua_pushstring(L, cpath);
    const char *launch = "./qtk/test/lua/launch.lua";
    r = luaL_loadfile(L, launch);
    if (LUA_OK != r) {
        wtk_debug("Cannot load %s : %s\r\n", launch, lua_tostring(L, -1));
        return -1;
    }

    while (demo->run) {
        qtk_sframe_msg_t *msg = method->pop(frame, -1);
        if (NULL == msg) continue;
        int type = method->get_type(msg);
        int signal = method->get_signal(msg);
        int id = method->get_id(msg);
        if (type == QTK_SFRAME_MSG_NOTICE) {
            switch (signal) {
            case QTK_SFRAME_SIGCONNECT:
                wtk_debug("socket %d connected\r\n", id);
                break;
            case QTK_SFRAME_SIGDISCONNECT: {
                qtk_sframe_msg_t *msg2 = method->new(method, QTK_SFRAME_MSG_CMD, id);
                method->set_cmd(msg2, QTK_SFRAME_CMD_CLOSE, NULL);
                method->push(method, msg2);
                wtk_debug("disconnected\r\n");
                break;
            }
            }
        } else if (type == QTK_SFRAME_MSG_PROTOBUF) {
            int lasttop = lua_gettop(L);
            lua_pushvalue(L, -1);
            lua_pushcfunction(L, lencode);
            lua_pushcfunction(L, ldecode);
            lua_pushlightuserdata(L, method->get_pb_var(msg));
            r = lua_pcall(L, 3, 1, 1);
            if (LUA_OK != r) {
                wtk_debug("lua load error : %s\r\n", lua_tostring(L, -1));
                return -1;
            }
            qpb_variant_t *v = lua_touserdata(L, -1);
            assert(v);
            qtk_sframe_msg_t *msg2 = method->new(method, QTK_SFRAME_MSG_PROTOBUF, id);
            method->set_protobuf(msg2, v, method->get_ws(msg));
            method->push(method, msg2);
            lua_settop(L, lasttop);
        }
        method->delete(frame, msg);
    }
    lua_close(L);

    return 0;
}


test_demo_t* protobuf_new(void * frame) {
    test_demo_t *demo = wtk_malloc(sizeof(*demo));
    demo->frame = frame;
    demo->run = 0;
    wtk_thread_init(&demo->thread, demo_route, demo);

    return demo;
}


int protobuf_start(test_demo_t *demo) {
    demo->run = 1;
    wtk_thread_start(&demo->thread);

    return 0;
}


int protobuf_stop(test_demo_t *demo) {
    demo->run = 0;
    return 0;
}


int protobuf_delete(test_demo_t *demo) {
    wtk_thread_join(&demo->thread);
    wtk_thread_clean(&demo->thread);
    wtk_free(demo);
    return 0;
}
