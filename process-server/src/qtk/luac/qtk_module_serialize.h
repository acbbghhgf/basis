#ifndef QTK_LUAC_QTK_MODULE_SERIALZE_H
#define QTK_LUAC_QTK_MODULE_SERIALZE_H
#include <lua.h>

#ifdef __cplusplus
extern "C" {
#endif

int qtk_module_pack(lua_State *L);
int qtk_module_unpack(lua_State *L);

#ifdef __cplusplus
}
#endif


#endif
