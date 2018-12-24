#ifndef MAIN_QTK_ENV_H_
#define MAIN_QTK_ENV_H_

#include "third/lua5.3.4/src/lua.h"
#include "third/lua5.3.4/src/lualib.h"
#include "third/lua5.3.4/src/lauxlib.h"

void qtk_env_init();
int qtk_env_update(const char *cfn, int len);
void qtk_env_clean();
const char *qtk_optstring(const char *key, const char *opt);
lua_Number qtk_optnumber(const char *key, lua_Number opt);
int qtk_optboolean(const char *key, int opt);

#endif
