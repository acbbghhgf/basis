#include "third/lua5.3.4/src/lua.h"
#include "third/lua5.3.4/src/lualib.h"
#include "third/lua5.3.4/src/lauxlib.h"
#include "third/framework/src/qtk/websocket/qtk_ws_protobuf.h"

#define LUA_QPB_VARIANT_HANDLE    "qpb_variant_t *"
#define tolvariant(L, idx)      ((qpb_variant_t *)luaL_checkudata(L, idx, LUA_QPB_VARIANT_HANDLE))

#include <assert.h>

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


static int ldecode_msg(lua_State *L, qtk_protobuf_t *pb);
/*
 * @param 1 is field value(qpb_variant_t *)
 * @param 2 is field value type
 * will return correponding value at top of stack
 * */
static void _decode_field(lua_State *L, qtk_protobuf_t *pb)
{
    qpb_variant_t *v = lua_touserdata(L, -2);
    int fv_desc_type = lua_type(L, -1);
    if (fv_desc_type == LUA_TTABLE) {
        ldecode_msg(L, pb);
        return;
    }
    assert(LUA_TNUMBER == fv_desc_type);
    int desctype = lua_tointeger(L, -1);
    ldecode_var(L, v, desctype);
}

static void _decode_repeated_field(lua_State *L, qtk_protobuf_t *pb)
{
    int fv_idx = lua_gettop(L) - 1;
    int fv_desc_idx = lua_gettop(L);
    lua_newtable(L);
    int ret_idx = lua_gettop(L);
    qpb_variant_t *v = lua_touserdata(L, fv_idx);
    assert(v->type == QPB_CTYPE_ARRAY);
    qpb_variant_t *s = v->v.a->data;
    qpb_variant_t *e = s + v->v.a->len;
    int i;
    for (i=1; s<e; i++, s++) {
        assert(s->type);
        lua_pushlightuserdata(L, s);
        lua_pushvalue(L, fv_desc_idx);
        _decode_field(L, pb);
        lua_rawseti(L, ret_idx, i);
        lua_pop(L, 1);  // pop s
        lua_pop(L, 1);  // pop fv_desc_idx
    }
    lua_settop(L, ret_idx);
}

/*
 * @param 1 is qpb_variant_t
 * @param 2 is schema
 * */
static int ldecode_msg(lua_State *L, qtk_protobuf_t *pb) {
    qpb_variant_t *v = lua_touserdata(L, -2);
    if (QPB_CTYPE_DELIMITED == v->type) {
        qtk_pb_decode_message(pb, v);
    }
    int schema_idx = lua_gettop(L);
    lua_newtable(L);
    int ret_idx = lua_gettop(L);
    lua_pushnil(L);
    while (lua_next(L, schema_idx)) {
        int field_idx = lua_gettop(L) - 1;
        assert(LUA_TTABLE == lua_type(L, -1));  // {idx, type, label}
        int field_desc_idx = lua_gettop(L);
        lua_rawgeti(L, field_desc_idx, 1);
        int array_idx = lua_tointeger(L, -1);
        lua_pop(L, 1); // pop array_idx
        lua_rawgeti(L, field_desc_idx, 3);
        int label = lua_tointeger(L, -1);
        lua_pop(L, 1); // pop label
        qpb_variant_t *vi = qpb_var_array_get(v, array_idx);
        if (vi == NULL || vi->type == 0) {
            if (QPB_LABEL_REQUIRED == label) {
                wtk_debug("%s: Required But Not Found\n", lua_tostring(L, field_idx));
                goto err;
            }
            goto skip;
        }
        lua_pushlightuserdata(L, vi);
        lua_rawgeti(L, field_desc_idx, 2);
        if (label == QPB_LABEL_REPEATED) {
            _decode_repeated_field(L, pb);
        } else {
            _decode_field(L, pb);
        }

        lua_pushvalue(L, field_idx);
        lua_pushvalue(L, -2);
        lua_rawset(L, ret_idx);

        lua_pop(L, 1); // pop decoded field value
        lua_pop(L, 1); // pop field value type
        lua_pop(L, 1); // pop field value (qpb_variant_t *)
skip:
        lua_pop(L, 1);
    }

    lua_settop(L, ret_idx);
    return 1;
err:
    lua_settop(L, ret_idx);
    return 0;
}


static int ldecode(lua_State *L) {
    qtk_protobuf_t pb;
    qtk_protobuf_init2(&pb, NULL);
    int ret = ldecode_msg(L, &pb);
    qtk_protobuf_clean(&pb);
    return ret;
}


static int lencode_var(lua_State *L, qpb_variant_t *v, int desctype) {
    switch (desctype) {
    case QPB_DESCTYPE_BOOL:
        qpb_var_set_bool(v, lua_toboolean(L, -1));
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

static void lencode_msg(lua_State *L, qpb_variant_t *v);
/*
 * @param 1 is field value
 * @param 2 is field value description
 **/
static void _encode_field(lua_State *L, qpb_variant_t *v)
{
    int fv_desc_type = lua_type(L, -1);
    if (fv_desc_type == LUA_TTABLE) {
        v->v.a = qtk_array_new2(sizeof(qpb_variant_t), 8);
        v->type = QPB_CTYPE_MESSAGE;
        lencode_msg(L, v);
        return;
    }
    assert(LUA_TNUMBER == fv_desc_type);
    int desctype = lua_tointeger(L, -1);
    lua_pushvalue(L, -2);
    lencode_var(L, v, desctype);
    lua_pop(L, 1);
}

static void _encode_repeated_field(lua_State *L, qpb_variant_t *v)
{
    qpb_var_set_array(v, 8);
    int fv_idx = lua_gettop(L) - 1;
    int fv_desc_idx = lua_gettop(L);
    int i;
    for (i=1; LUA_TNIL != lua_rawgeti(L, fv_idx, i); i++) {
        qpb_variant_t tmp_variant;
        qpb_variant_init(&tmp_variant);
        lua_pushvalue(L, fv_desc_idx);
        _encode_field(L, &tmp_variant);
        qtk_array_set(v->v.a, i-1, &tmp_variant);
        lua_pop(L, 1); // pop field value description
        lua_pop(L, 1); // pop field value
    }
    v->type = QPB_CTYPE_ARRAY;
    lua_settop(L, fv_desc_idx);
}

/*
 * @param 1 is data (table, idx is -2)
 * @param 2 is schema (table, idx is -1)
 * top 2 elements in stack is args
 **/
static void lencode_msg(lua_State *L, qpb_variant_t *v) {
    luaL_checktype(L, -2, LUA_TTABLE);
    luaL_checktype(L, -1, LUA_TTABLE);

    int data_idx = lua_gettop(L) -1;
    int schema_idx = lua_gettop(L);

    lua_pushnil(L);
    while (lua_next(L, schema_idx)) {  // iter schema table
        int field_idx = lua_gettop(L) - 1;
        assert(LUA_TTABLE == lua_type(L, -1));  // {idx, type, label}
        int field_desc_idx = lua_gettop(L);
        lua_rawgeti(L, field_desc_idx, 1);
        int array_idx = lua_tointeger(L, -1);
        lua_pop(L, 1); // pop array_idx
        lua_rawgeti(L, field_desc_idx, 3);
        int label = lua_tointeger(L, -1);
        lua_pop(L, 1); // pop label

        lua_pushvalue(L, field_idx);
        int field_type = lua_rawget(L, data_idx);
        if (field_type == LUA_TNIL) {
            if (label == QPB_LABEL_REQUIRED) {
                wtk_debug("%s: Required But Not Found\n", lua_tostring(L, field_idx));
                goto err;
            }
            lua_pop(L, 1);
            goto skip;
        }
        lua_rawgeti(L, field_desc_idx, 2);

        qpb_variant_t tmp_variant;
        qpb_variant_init(&tmp_variant);
        if (label == QPB_LABEL_REPEATED) {
            _encode_repeated_field(L, &tmp_variant);
        } else {
            _encode_field(L, &tmp_variant);
        }
        qtk_array_set(v->v.a, array_idx, &tmp_variant);
        lua_pop(L, 1); // pop field value
        lua_pop(L, 1); // pop field value type
skip:
        lua_pop(L, 1); // pop field_desc_idx
    }
    lua_settop(L, schema_idx);
    v->type = QPB_CTYPE_MESSAGE;
    return ;
err:
    lua_settop(L, schema_idx);
    qpb_variant_clean(v);
}

static const luaL_Reg variantlib[] = {
    {NULL, NULL}
};

static void createmeta(lua_State *L)
{
    luaL_newmetatable(L, LUA_QPB_VARIANT_HANDLE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, variantlib, 0);
    lua_pop(L, 1);
}

/*
 * @param 1 is data (table)
 * @param 2 is schema (table)
 * Use ret value very carefully, it should free outside in
 * C func, or it will cause mem leak
 **/
static int lencode(lua_State *L) {
    qpb_variant_t *v = qpb_variant_new();
    v->v.a = qtk_array_new2(sizeof(qpb_variant_t), 8);
    v->type = QPB_CTYPE_MESSAGE;
    lencode_msg(L, v);
    lua_pushlightuserdata(L, v);
    luaL_setmetatable(L, LUA_QPB_VARIANT_HANDLE);
    return 1;
}

static const luaL_Reg mlib[] = {
    {"pack", lencode},
    {"unpack", ldecode},
    {NULL,  NULL}
};

LUAMOD_API int luaopen_protobuf(lua_State *L)
{
    luaL_newlib(L, mlib);
    createmeta(L);
    return 1;
}
