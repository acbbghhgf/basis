//#ifndef UNUSE_SQL
//#define USE_SQL
//#endif
#ifdef USE_SQL
#ifndef WTK_UTIL_WTK_SQLITE_H_
#define WTK_UTIL_WTK_SQLITE_H_
#include "wtk/core/wtk_str.h"
#include "third/sqlite/sqlite3.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wtk_array.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_sqlite wtk_sqlite_t;
typedef int (*wtk_sqlite_add_param_f)(void *ths,sqlite3_stmt *stmt);
typedef void (*wtk_sqlite_notify_value_f)(void *ths,int index,int col,sqlite3_value *v);

/**
 * when return 0 jump notify where mulit result
 */
typedef int (*wtk_sqlite_notify_value2_f)(void *ths,int index,int col,sqlite3_value *v);
#define wtk_sqlite_select_str1_s(s,str,p,p_len) wtk_sqlite_select_str1(s,str,sizeof(str)-1,p,p_len)
#define wtk_sqlite_select_str_s(s,a,str) wtk_sqlite_select_str(s,a,str,sizeof(str)-1)
#define wtk_sqlite_exe_s(s,sql) wtk_sqlite_exe(s,sql,sizeof(sql)-1)
#define wtk_sqlite_exe2_s(s,str,p,p_len) wtk_sqlite_exe2(s,str,sizeof(str)-1,p,p_len)

struct wtk_sqlite
{
	sqlite3* db;
	wtk_strbuf_t *buf;
	sqlite3_stmt* trans_stmt;
};

wtk_sqlite_t* wtk_sqlite_new(char *fn);
int wtk_sqlite_delete(wtk_sqlite_t *s);
int wtk_sqlite_init(wtk_sqlite_t *s,char *fn);
int wtk_sqlite_clean(wtk_sqlite_t *s);
int wtk_sqlite_select_str1(wtk_sqlite_t* s,char *sql,int sql_len,char *p,int p_len);
int wtk_sqlite_select_str(wtk_sqlite_t* s,wtk_array_t *a,char *sql,int sql_len);
int wtk_sqlite_exe(wtk_sqlite_t* s,char *sql,int sql_len);
int wtk_sqlite_exe2(wtk_sqlite_t* s,char *sql,int sql_len,char *p,int p_len);
int wtk_sqlite_exe3(wtk_sqlite_t *s,wtk_string_t *sql,void *ths,wtk_sqlite_add_param_f add_param,wtk_sqlite_notify_value_f notify);
int wtk_sqlite_exe4(wtk_sqlite_t *s,wtk_string_t *sql,void *ths,wtk_sqlite_add_param_f add_param,wtk_sqlite_notify_value2_f notify);
void wtk_sqlite_print(wtk_sqlite_t *s);


int wtk_sqlite_begin_transaction(wtk_sqlite_t *s,wtk_string_t *sql);
int wtk_sqlite_exe_transaction(wtk_sqlite_t *s,void *ths,wtk_sqlite_add_param_f add_param,wtk_sqlite_notify_value2_f notify);
int wtk_sqlite_end_transaction(wtk_sqlite_t *s);
//---------------------------------- test section------------------
void wtk_sqlite_add_param_notify_test(void *ths,int index,int col,sqlite3_value *v);
#ifdef __cplusplus
};
#endif
#endif
#endif
