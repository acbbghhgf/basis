#ifndef QTK_ZEUS_QTK_ZENV_H
#define QTK_ZEUS_QTK_ZENV_H
#include "lua.h"
#include "wtk/os/wtk_lock.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct qtk_zenv qtk_zenv_t;

struct qtk_zenv {
    wtk_lock_t lock;
    lua_State *L;
};

qtk_zenv_t* qtk_zenv_new();
int qtk_zenv_delete(qtk_zenv_t* zenv);
const char* qtk_zenv_get(qtk_zenv_t *zenv, const char *key);
void qtk_zenv_set(qtk_zenv_t *zenv, const char *key, const char *value);

#ifdef __cplusplus
}
#endif


#endif
