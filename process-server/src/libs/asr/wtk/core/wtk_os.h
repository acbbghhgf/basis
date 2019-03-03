#ifndef WLIB_CORE_WTK_INIT_H_
#define WLIB_CORE_WTK_INIT_H_
#include <stdio.h>
#include "wtk_type.h"
#include "wtk_str.h"
#include "wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
#ifdef WIN32
#include <stdlib.h>
#ifdef WINCE
#include <ceconfig.h>
#else
#include <direct.h>
#include <io.h>
#endif

#define F_OK 0
#define R_OK 4
#define W_OK 2
#define wtk_file_access(fn,m) _access(fn,m)
#define wtk_file_exist(fn) wtk_file_access(fn,F_OK)
#define wtk_file_readable(fn) wtk_file_access(fn,R_OK)
#define wtk_file_writeable(fn) wtk_file_access(fn,W_OK)
typedef int (*wtk_dir_walk_handler_t)(void* ,char *fn);
//void dir_monitor(const char* path,  dir_monitor_handler cb,void* user_data,HANDLE* handle);
int wtk_dir_walk(const char* path,wtk_dir_walk_handler_t cb,void* user_data);

int wtk_is_dir(char *dn);
#else
/**
 * @brief return 0 on success.
 */
#define wtk_file_access(fn,m) access(fn,m)
#define wtk_file_exist(fn) wtk_file_access(fn,F_OK)
#define wtk_file_readable(fn) wtk_file_access(fn,R_OK)
#define wtk_file_writeable(fn) wtk_file_access(fn,W_OK)
#define wtk_file_exeable(fn) wtk_file_access(fn,X_OK)
int wtk_is_dir(char *dn);
#endif
#ifdef WIN32
#define DIR_SEP '\\'
#define DIR_SEP1 '/'
#else
#define DIR_SEP '/'
#endif

typedef int(*wtk_dir_walk_f)(void *ths,wtk_string_t *fn);
#if defined __IPHONE_OS__
#else
int wtk_dir_walk(char *dir,void *ths,wtk_dir_walk_f walk);

typedef int(*wtk_dir_walk2_f)(void *ths,char *fn);
int wtk_dir_walk2(char *dir,void *ths,wtk_dir_walk2_f walk);
#endif



//------------------------ directory and file section ---------------------------
/**
 * @brief create directory;
 */
int wtk_mkdir(char* dn);

/**
 * @brief create directory and parent directory if not created;
 */
int wtk_mkdir_p(char* fn,char sep,int create_last_entry);

/**
 * @brief remove . and .. from fn, and save the result into buf;
 * @param fn like /home/lz123/wtk/wvite/httpa/8.9/src/../ext/../html to remove ..
 * @param fn_bytes bytes of fn;
 * @param buf to save the result;
 * @param sep use to separate directory: '/','\';
 */
void wtk_real_fn(char *fn,int fn_bytes,wtk_strbuf_t *buf,char sep);
#define wtk_real_fn_s(fn,buf,sep) wtk_real_fn(fn,sizeof(fn)-1,buf,sep)


/**
 * @directory name for fn;
 */
wtk_string_t* wtk_dir_name(char *fn,char sep);

wtk_string_t wtk_dir_name2(char *data,int len,char sep);

/**
 * @brief realpath of fn;
 * @return char* must be freed;
 */
char* wtk_realpath(char *fn,char *buf);

wtk_string_t* wtk_dirname(char *fn,char sep);
wtk_string_t* wtk_str_left(char *fn,int len,char sep);
wtk_string_t* wtk_basename(char* fn,char sep);
wtk_string_t* wtk_str_right(char* fn,int len,char sep);
wtk_string_t* wtk_real_dirname(char *fn);
FILE* wtk_file_open(char* fn,char * mode);

//------------------------ file section ----------------------------------------
uint64_t file_length(FILE *f);
uint64_t wtk_file_size(char *fn);

char* file_read_buf(char* fn, int *n);
char* file_read_buf2(char* fn, int fn_len,int *n);
int file_write_buf(char* fn, const char* data, size_t len);

//------------------------ time section ----------------------------------------
double time_get_ms();
double time_get_cpu();
void wtk_msleep(int ms);

//----------------------- compile section --------------------------------------
int wtk_gcc_month();
int wtk_gcc_day();
int wtk_gcc_year();
int wtk_get_build_timestamp(char *buf);
int wtk_os_timestamp(char *buf);
int wtk_os_timestamp2(wtk_strbuf_t *buf);

//------------------------- --------------------------------------------------
char* wtk_search_file(char *fn,wtk_string_t **path,int n,wtk_strbuf_t *buf);
unsigned long long  wtk_file_lines(char *fn);
int wtk_file_read_line(FILE *f,wtk_strbuf_t *buf,unsigned long long index,char *tmp,int tmp_size);
int wtk_file_read_line2(FILE *f,wtk_strbuf_t *buf,unsigned long long index);

int wtk_file_copy(char *src,char *dst,char sep);
int wtk_file_copy2(FILE *fin,char *dst,char sep,int want_len);

//[left,right];
int wtk_random(int left,int right);


typedef void(*wtk_os_dir_walk_notify_f)(void *ths,char *name,int len);
int wtk_os_dir_walk(char *dir,void *ths,wtk_os_dir_walk_notify_f notify);

#ifdef __cplusplus
};
#endif
#endif
