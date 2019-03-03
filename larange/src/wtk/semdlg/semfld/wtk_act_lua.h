#ifndef WTK_LEX_ACT_WTK_ACT_LUA_H_
#define WTK_LEX_ACT_WTK_ACT_LUA_H_
#include "wtk/core/wtk_type.h"
#include "wtk/lua/wtk_lua2.h"
#include "wtk/core/json/wtk_json.h"
#include "wtk/semdlg/semfld/wtk_semslot.h"
#include "wtk/core/wtk_str_parser.h"
#ifdef __cplusplus
extern "C" {
#endif
#define wtk_act_get_str_value_s(act,k)  wtk_act_get_str_value(act,k,sizeof(k)-1)
#define wtk_json_act_get_value_s(act,k) wtk_json_act_get_value(act,k,sizeof(k)-1)


typedef struct
{
	wtk_heap_t *heap;
	wtk_json_item_t *json;
	wtk_json_item_t *req;
}wtk_json_act_t;

typedef enum
{
	WTK_ACT_JSON,
	WTK_ACT_SLOT,
	WTK_ACT_HASH,
}wtk_act_type_t;

typedef struct
{
	union {
		wtk_json_act_t json;
		wtk_semslot_t *slot;
		wtk_str_hash_t *hash;
	}v;
	//unsigned use_json:1;
	wtk_act_type_t type;
}wtk_act_t;

void wtk_act_init_json(wtk_act_t *act,wtk_heap_t *heap);
void wtk_act_init_slot(wtk_act_t *act,wtk_semslot_t *slot);
void wtk_act_init_hash(wtk_act_t *act,wtk_str_hash_t *hash);
wtk_string_t* wtk_act_json_get_str_value(wtk_json_item_t *req);
wtk_json_item_t* wtk_json_act_get_value(wtk_json_act_t *act,char *k,int k_len);
int wtk_act_nslot(wtk_act_t *act);
void wtk_act_reset(wtk_act_t *act);
void wtk_act_update(wtk_act_t *act);
void wtk_act_print(wtk_act_t *act);
wtk_string_t* wtk_act_get_str_value(wtk_act_t *act,char *k,int k_len);
int wtk_act_has_key(wtk_act_t *a,char *k,int k_len);
int wtk_act_is_str_value(wtk_act_t *a,char *k,int k_len);
void wtk_lua_push_json(lua_State *l,wtk_json_item_t *item);

int wtk_lua_act_print(lua_State *l);
int wtk_lua_act_get(lua_State *l);
int wtk_lua_act_set(lua_State *l);
int wtk_lua_chn2number(lua_State *l);
int wtk_lua_chnmap(lua_State *l);
int wtk_lua_str2array(lua_State *l);
int wtk_lua_itochn(lua_State *l);
void wtk_act_lua_link(wtk_lua2_t *lua2);


wtk_act_t* wtk_act_new_str(char *data,int len);
void wtk_act_delete(wtk_act_t *act);
#ifdef __cplusplus
};
#endif
#endif
