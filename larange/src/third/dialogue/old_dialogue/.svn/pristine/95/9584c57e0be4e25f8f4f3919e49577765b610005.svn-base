#ifndef WTK_SEMDLG_WTK_SEMLUA
#define WTK_SEMDLG_WTK_SEMLUA
#include "wtk/core/wtk_type.h" 
#include "wtk/lua/wtk_lua2.h"
#include "wtk/core/json/wtk_json.h"
#include "wtk/core/wtk_calendar.h"
#ifdef __cplusplus
extern "C" {
#endif

int wtk_lua_semfld_set_output(lua_State *l);
int wtk_lua_semfld_get_output(lua_State *l);
int wtk_lua_semfld_set_slot(lua_State *l);
int wtk_lua_semfld_set_slot2(lua_State *l);
int wtk_lua_semfld_del_slot(lua_State *l);
int wtk_lua_semfld_del_slot2(lua_State *l);
int wtk_lua_semfld_get_slot(lua_State *l);
int wtk_lua_semfld_reset_slot(lua_State *l);
int wtk_lua_semfld_get_ask_slot(lua_State *l);
int wtk_lua_semfld_get_ask_slot_value(lua_State *l);
int wtk_lua_semfld_set_ask_slot(lua_State *l);
int wtk_lua_semfld_set_end(lua_State *l);

/*
	使用系统slot解析nlg  sys_answer()
	*/
int wtk_lua_semfld_process_nlg(lua_State *l);

/*
	使用参数解析nlg  sys_answer(a="b")
*/
int wtk_lua_semfld_process_nlg2(lua_State *l);

/**
 * 使用参数解析nlg  sys_answer(a="b") return string
 */
int wtk_lua_semfld_process_nlg3(lua_State *l);
int wtk_lua_semfld_get_dn(lua_State *l);

int wtk_lua_semdlg_get_fld(lua_State *l);
int wtk_lua_semdlg_set_next_fld(lua_State *l);
int wtk_lua_semdlg_get_input(lua_State *l);
int wtk_lua_semdlg_process(lua_State *l);
int wtk_lua_semdlg_has_process_domain(lua_State *l);

/*
 * wtk_semdlg_set_ext_json(dlg,"cmd",tbl);
 */
int wtk_lua_semdlg_set_ext_json(lua_State *l);

int wtk_lua_semdlg_exe(lua_State *l);

/**
 * set pre output
 */
int wtk_lua_semdlg_set_output(lua_State *l);
int wtk_lua_semdlg_syn(lua_State *l);
int wtk_lua_semdlg_play_file(lua_State *l);
int wtk_lua_semdlg_set_rec_grammar(lua_State *l);
int wtk_lua_semdlg_get_conf(lua_State *l);
int wtk_lua_semdlg_get_vadtime(lua_State *l);
int wtk_lua_semdlg_kv_get(lua_State *l);
int wtk_lua_semdlg_kv_set(lua_State *l);

void wtk_lua_semfld_link(wtk_lua2_t *lua2);
void wtk_lua_semfld_link_wtk(wtk_lua2_t *lua2);
#ifdef __cplusplus
};
#endif
#endif
