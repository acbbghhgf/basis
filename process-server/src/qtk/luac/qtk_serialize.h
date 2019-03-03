#ifndef QTK_LUAC_QTK_SERIALIZE_H
#define QTK_LUAC_QTK_SERIALIZE_H
#include <lua.h>

#ifdef __cplusplus
extern "C" {
#endif

int qtk_lua_pack(lua_State *L);
int qtk_lua_unpack(lua_State *L);


#ifdef __cplusplus
}
#endif

#endif
