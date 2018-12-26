#ifndef WTK_LUA_WTK_LUA2_H_
#define WTK_LUA_WTK_LUA2_H_
#include "wtk/core/wtk_type.h"
//#include "wtk/core/param/wtk_param.h"
#include "third/lua-5.1.4/src/lua.h"
#include "third/lua-5.1.4/src/lualib.h"
#include "third/lua-5.1.4/src/lauxlib.h"
#include "wtk_lua2_cfg.h"
#include "wtk/core/rbin/wtk_rbin2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lua2 wtk_lua2_t;

typedef enum
{
	WTK_LUA2_NUMBER,
	WTK_LUA2_STRING,
	WTK_LUA2_THS,
	WTK_LUA2_PUSH_VALUE,
}wtk_lua2_arg_type_t;

typedef void (*wtk_lua2_push_value_f)(lua_State *l,void *ths);

typedef struct
{
	wtk_lua2_arg_type_t type;
	union
	{
		double number;
		wtk_string_t str;
		void *ths;
		struct {
			void *ths;
			wtk_lua2_push_value_f push;
		}push_value;
	}v;
}wtk_lua2_arg_t;

struct wtk_lua2
{
	wtk_lua2_cfg_t *cfg;
	lua_State *state;
	wtk_rbin2_t *rbin;
};

wtk_lua2_t* wtk_lua2_new(wtk_lua2_cfg_t *cfg,wtk_rbin2_t *rbin);
void wtk_lua2_delete(wtk_lua2_t *lua);
void wtk_lua2_gc(wtk_lua2_t *lua);
int wtk_lua2_bytes(wtk_lua2_t *lua);
int wtk_lua2_load(wtk_lua2_t *lua,char *data,int bytes);
int wtk_lua2_load_file2(wtk_lua2_t *lua2,char *fn);
int wtk_lua2_load_str(wtk_lua2_t *lua2,char *data,int bytes);
int wtk_lua2_process_number(wtk_lua2_t *lua,char *post_func,float arg,float *result);

/**
 * @brief return result is string and saved to result
 */
int wtk_lua2_process_string(wtk_lua2_t *lua,char *post_func,char *arg,int arg_bytes,wtk_strbuf_t *result);

/**
 * @brief return result is string and saved to result, and param is wtk_string_t as param,with null end;
 */
int wtk_lua2_process_string2(wtk_lua2_t *lua,char *post_func,wtk_strbuf_t *result,...);

void wtk_lua2_link_function(wtk_lua2_t *lua,lua_CFunction func,char *name);

/**
 *	wtk_lua2_arg_t arg;
 */
int wtk_lua2_process_arg(wtk_lua2_t *lua,char *func,wtk_strbuf_t *result,...);

#ifdef __cplusplus
};
#endif
#endif
