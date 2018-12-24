#include "third/lua5.3.4/src/lua.h"
#include "third/lua5.3.4/src/lauxlib.h"

#include "qtk/sframe/qtk_sframe.h"
#include "qtk/http/qtk_request.h"
#include "main/qtk_bifrost_main.h"
#include "main/qtk_context.h"
#include "main/qtk_xfer.h"
#include "third/framework/src/qtk/websocket/qtk_ws_protobuf.h"

#define GET_METHOD()  qtk_bifrost_self()->gate
#define GET_BIFROST_UPLOAD() qtk_bifrost_self()->up


static int _method_atoi(const char *method, int len)
{
    switch(len) {
    case 3:
        if (0 == memcmp(method, "GET", 3))
            return HTTP_GET;
        if (0 == memcmp(method, "PUT", 3))
            return HTTP_PUT;
        break;
    case 4:
        if (0 == memcmp(method, "POST", 4))
            return HTTP_POST;
        if (0 == memcmp(method, "HEAD", 4))
            return HTTP_HEAD;
        break;
    case 6:
        if (0 == memcmp(method, "DELETE", 6))
            return HTTP_DELETE;
        break;
    default:
        return HTTP_UNKNOWN;
    }
    return HTTP_UNKNOWN;
}
/* @param 1 is method(string)
 * @param 2 is url for REQUEST
 * @param 3 is content(table) 
 * @return value true if xfer ok or nil
 * */
// static int lupload(lua_State *L){   //写入通过清零方式写入，如果当前没有通过创建方式创建一个文件进行操作
// //O_TRUNC 若文件存在并且以可写的方式打开时, 此旗标会令文件长度清为0, 而原来存于该文件的资料也会消失
//     int fil_num;
//     qtk_bifrost_upload_queue_t *item;
//     size_t len;
//     ssize_t ret_len;
//     char buf_tail[BUFSIZ];
//     memset(buf_tail, 0, sizeof(buf_tail));
    
//     wtk_queue_t *q_write = GET_BIFROST_UPLOAD()->upload_q_write;
//     wtk_queue_t *q_read = GET_BIFROST_UPLOAD()->upload_q_read;
//     wtk_queue_node_t *node = wtk_queue_pop(q_write);
    
//     if(!node){        
//         item = wtk_malloc(sizeof(*item));
//         snprintf(item->filename, sizeof(item->filename), "%s", wtk_itoa((int)time(0))); //以当前时间命名        
//         fil_num = open(item->filename, O_RDWR | O_APPEND | O_CREAT |O_TRUNC, S_IRWXU);
//         item->file = fil_num;
//         if(!item->file){
//             wtk_debug("open file fail\n");
//             wtk_debug("111\n");
//             goto err;
//         }
//     }
    
//     else{
        
//         item = data_offset2(node, qtk_bifrost_upload_queue_t, q_n);
//         if(!item->filename){
//             wtk_debug("filename NULL\n");
//             goto err;
//         }
//         fil_num= open(item->filename, O_RDWR | O_APPEND | O_CREAT |O_TRUNC, S_IRWXU);
//         item->file = fil_num;
//         if(!item->file){
//             wtk_debug("open file fail\n");
//             wtk_debug("22\n");
//             goto err;
//         }
//     }
//     if (LUA_TSTRING == lua_type(L, 1)) {
//         const char *msgbody = lua_tolstring(L, 1, &len);
//         ret_len = write(fil_num, msgbody, len);
//         if(ret_len == -1){
//             wtk_debug("write fail\n");
//             perror("write fail");
//             goto err;
//         }
//     }
//     wtk_queue_push(q_read, &(item->q_n));
//     close(item->file);
//     lua_pop(L, 1);
//     lua_pushboolean(L, 1);
//     return 1;
// err:
//     printf("upload error\n");
//     return 0;
// }

/* @param 1 is method(string) or status(number)
 * @param 2 is content(table) for RESPONSE or url for REQUEST
 * @param 3 is content(table) for REQUEST or nil for RESPONSE
 * @return value true if xfer ok or nil
 * */
static int lxfer(lua_State *L)
{
    int ret, id;
    size_t len;
    qtk_sframe_msg_t *msg = NULL;
    qtk_sframe_method_t *method = GET_METHOD();
    if (LUA_TSTRING == lua_type(L, 1)) {
        luaL_checktype(L, 2, LUA_TSTRING);
        luaL_checktype(L, 3, LUA_TTABLE);
        const char *meth = lua_tolstring(L, 1, &len);
        int meth_num = _method_atoi(meth, len);
        if (HTTP_UNKNOWN == meth_num) goto err;
        const char *url = lua_tostring(L, 2);
        lua_pushliteral(L, "addr");
        ret = lua_gettable(L, -2);
        if (ret != LUA_TNUMBER) {
            printf("req addr not number\n");
            lua_pop(L, 1);
            goto err;
        }
        id = lua_tonumber(L, -1);
        lua_pop(L, 1);
        msg = method->new(method, QTK_SFRAME_MSG_REQUEST, id);
        method->set_url(msg, url, strlen(url));
        method->set_method(msg, meth_num);
    } else if (LUA_TNUMBER == lua_type(L, 1)) {
        int status = lua_tonumber(L, 1);
        luaL_checktype(L, 2, LUA_TTABLE);
        lua_pushliteral(L, "addr");
        ret = lua_gettable(L, -2);
        if (ret != LUA_TNUMBER) {
            printf("rep addr not number, %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
            goto err;
        }
        id = lua_tonumber(L, -1);
        if (id < 0) goto err;
        lua_pop(L, 1);
        msg = method->new(method, QTK_SFRAME_MSG_RESPONSE, id);
        method->set_status(msg, status);
    } else{
        printf("err 1st param type\n");
        goto err;
    }
    lua_pushliteral(L, "body");
    ret = lua_gettable(L, -2);
    if (ret == LUA_TSTRING) {
        const char *body = lua_tolstring(L, -1, &len);
        method->add_body(msg, body, len);
    }
    lua_pop(L, 1);
    lua_pushliteral(L, "field");
    ret = lua_gettable(L, -2);
    if (ret == LUA_TTABLE) {
        lua_pushnil(L);
        while(lua_next(L, -2)) {
            const char *key = lua_tostring(L, -2);
            const char *value = lua_tostring(L, -1);
            if (key && value)
                method->add_header(msg, key, value);
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    method->push(method, msg);
    lua_pushboolean(L, 1);
    return 1;
err:
    printf("xfer error\n");
    return 0;
}

static int _recon_atoi(const char *recon, int *succ)
{
    *succ = 1;
    if (0 == strcmp(recon, "LAZY_RECONNECT"))
        return QTK_SFRAME_RECONNECT_LAZY;
    if (0 == strcmp(recon, "NOT_RECONNECT"))
        return QTK_SFRAME_NOT_RECONNECT;
    if (0 == strcmp(recon, "NOW_RECONNECT"))
        return QTK_SFRAME_RECONNECT_NOW;
    *succ = 0;
    return 0;
}

/* @param 1 is addr we request to establish
 * @param 2 is reconnect(string)
 * @return value is id
 * */
static int lestb(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TSTRING);
    const char *addr = lua_tostring(L, 1);
    const char *recon = lua_tostring(L, 2);
    int succ;
    int recon_num = _recon_atoi(recon, &succ);
    if (!succ) return 0;
    qtk_sframe_method_t *method = GET_METHOD();
    int id = qtk_xfer_estab(method, addr, recon_num);
    if (id < 0) return 0;
    lua_pushinteger(L, id);
    return 1;
}

static int llisten(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TSTRING);
    int backlog = luaL_checkinteger(L, 2);
    const char *addr = lua_tostring(L, 1);
    qtk_sframe_method_t *method = GET_METHOD();
    int id = qtk_xfer_listen(method, addr, backlog);
    if (id < 0) return 0;
    lua_pushinteger(L, id);
    return 1;
}

static int lnewPool(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TSTRING);
    int nslot = luaL_checkinteger(L, 2);
    const char *addr = lua_tostring(L, 1);
    qtk_sframe_method_t *method = GET_METHOD();
    int id = qtk_xfer_newPool(method, addr, nslot);
    if (id < 0) return 0;
    lua_pushinteger(L, -id);
    return 1;
}

/* @param 1 is addr
 * @return value is true is push succ or nil
 * */
static int ldestroy(lua_State *L)
{
    lua_Integer id = luaL_checkinteger(L, 1);
    qtk_sframe_method_t *method = GET_METHOD();
    qtk_xfer_destroy(method, id);
    lua_pushboolean(L, 1);
    return 1;
}

/* @param1 is qpb_variant_t
 * @param2 is addr
 * @param3 is wsi
 * */
static int lsendPB(lua_State *L)
{
    qpb_variant_t *v = luaL_checkudata(L, 1, "qpb_variant_t *"); // check lualib-src/lua_protobuf.c
    lua_Integer id = luaL_checkinteger(L, 2);
    struct lws *wsi = lua_touserdata(L, 3);
    qtk_sframe_method_t *method = GET_METHOD();
    qtk_sframe_msg_t *msg = method->new(method, QTK_SFRAME_MSG_PROTOBUF, id);
    method->set_protobuf(msg, v, wsi);
    method->push(method, msg);
    lua_pushboolean(L, 1);
    return 1;
}

LUAMOD_API int luaopen_senddata(lua_State *L)
{
	luaL_checkversion(L);

	luaL_Reg l[] = {
		{ "xfer"    , lxfer },
//        {"upload"   , lupload},
        { "sendPB"  , lsendPB},
		{ "estb"    , lestb },
        { "destroy" , ldestroy },
        { "listen"  , llisten},
        { "newPool" , lnewPool},
        { NULL      , NULL },
	};

	luaL_newlibtable(L, l);

	lua_getfield(L, LUA_REGISTRYINDEX, "qtk_context");
	qtk_context_t *ctx = lua_touserdata(L,-1);
	if (ctx == NULL) {
		//return luaL_error(L, "Init bifrost context first");
	}

	luaL_setfuncs(L, l, 1);

	return 1;
}
