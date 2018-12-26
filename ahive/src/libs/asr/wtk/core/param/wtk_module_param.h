#ifndef WTK_SPEECH_LIB_WTK_MODULE_PARAM_H_
#define WTK_SPEECH_LIB_WTK_MODULE_PARAM_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk_param.h"
#include "wtk_module_macro.h"
#include "wtk/core/wtk_audio_type.h"
#ifdef __cplusplus
extern "C" {
#endif
#define wtk_param_arg(t,n,cn) {t,n,sizeof(n)-1,cn}
typedef struct wtk_param_arg wtk_param_arg_t;
typedef struct wtk_module_arg wtk_module_arg_t;
typedef struct wtk_module_param wtk_module_param_t;

typedef wtk_param_t* (*wtk_module_param_read_f)(void *data,wtk_param_arg_t *arg);
typedef void* (*wtk_module_lib_func_find_f) (char* name);
typedef int (*wtk_module_dynamic_init_f)();
typedef int (*wtk_module_static_init_f)(wtk_local_cfg_t *cfg,void **module_data);
typedef int (*wtk_module_static_release_f)(void *module_data);
typedef int (*wtk_module_dynamic_release_f)();
typedef wtk_module_arg_t* (*wtk_module_lib_get_arg_f)();
typedef wtk_module_arg_t* (*wtk_module_engine_get_arg_f)(void *module_data,wtk_string_t *engine_name);
typedef void* (*wtk_module_dynamic_engine_new_f)(int param_num,char** p);
typedef void* (*wtk_module_static_engine_new_f)(void *module_data,wtk_local_cfg_t *cfg);
typedef int (*wtk_module_engine_delete_f)(void* engine);
typedef void* (*wtk_module_instance_new_f)(void* engine);
typedef int (*wtk_module_instance_delete_f)(void* engine,void* handle);

/**
 * ret_array[0] is WTK_NUMBER refrence, the stream state.
 * ret_array[1] is the result or information.
 */
typedef int (*wtk_module_instance_calc_f)(void* handle,wtk_module_param_t *param,wtk_param_t* ret_array);

typedef void (*wtk_module_write_f)(void *usr_data,const wtk_param_t *param);


typedef enum
{
	WTK_MODULE_STREAM_ERR=-1,
	WTK_MODULE_STREAM_APPENDED=0,
	WTK_MODULE_STREAM_END=1
}wtk_module_stream_state_t;

typedef struct
{
	char* name;
	Pointer func;
}wtk_func_map_t;

struct wtk_param_arg
{
	wtk_paramtype_t	type;
	char* name;
	int name_len;
	int can_be_nil;
};

typedef struct
{
	wtk_param_arg_t *args;
	int len;
}wtk_func_arg_t;

struct wtk_module_arg
{
	wtk_func_arg_t start;
	wtk_func_arg_t append;
	wtk_func_arg_t end;
};

typedef struct
{
	wtk_param_t **params;
	int len;
	int valid;
}wtk_func_param_t;

typedef enum
{
	WTK_SPEECH_START=1,
//#define WTK_SPEECH_START 1
	WTK_SPEECH_APPEND=2,
//#define WTK_SPEECH_APPEND 2
	WTK_SPEECH_END=4,
//#define WTK_SPEECH_END 4
}wtk_speech_state_t;

typedef struct
{
	int type; //see wtk_audio_type_t
	int channel;
	int samplerate;
	int samplesize;		//bytes;
}wtk_audio_tag_t;

struct wtk_module_param
{
	int state;	//000| 1st bit for start or not; 2st bit for append or not; 3st bit for end or not; wtk_speech_state_t
	wtk_audio_tag_t audio_tag;
	wtk_func_param_t start;
	wtk_func_param_t append;
	wtk_func_param_t end;
	//================== httpd2 used for interactive ==============
	void *app_data;				//used for attach application data;current for share stk
	void *write_usr_data;
	wtk_module_write_f write;
};

wtk_param_t* wtk_param_new_ret_array();
int wtk_param_delete_ret_array(wtk_param_t *ret_array);
void wtk_param_set_ret_array(wtk_param_t *ret_array,int no,wtk_param_t *info);

int wtk_func_param_init(wtk_func_param_t *param,wtk_func_arg_t *arg);
int wtk_func_param_reset(wtk_func_param_t *param);
int wtk_func_param_clean(wtk_func_param_t *param);
int wtk_func_param_update(wtk_func_param_t *p,wtk_func_arg_t *arg,wtk_module_param_read_f read,void *data);
int wtk_func_param_release_param(wtk_func_param_t *param);

int wtk_module_param_init(wtk_module_param_t *param,wtk_module_arg_t *arg);
int wtk_module_param_clean(wtk_module_param_t *param);
int wtk_module_param_reset(wtk_module_param_t *param);
int wtk_module_param_release_param(wtk_module_param_t *param);
int wtk_module_param_fill(wtk_module_param_t *mp,wtk_module_arg_t *ma,
		int state,void *read_data,wtk_module_param_read_f read);
int wtk_func_arg_from_lc(wtk_func_arg_t *arg,wtk_local_cfg_t *main);
void wtk_func_arg_clean(wtk_func_arg_t *arg);

void wtk_module_arg_print(wtk_module_arg_t* arg);
void wtk_param_arg_print(wtk_param_arg_t *arg);
void wtk_func_arg_print(wtk_func_arg_t *arg);
void wtk_module_param_print(wtk_module_param_t *p);
void wtk_func_param_print(wtk_func_param_t *p);

int wtk_audio_tag_need_pre(wtk_audio_tag_t *tag,int dst_rate);
void wtk_audio_tag_reset(wtk_audio_tag_t *tag);
void wtk_audio_tag_update_local(wtk_audio_tag_t *tag,wtk_local_cfg_t *lc);
void wtk_audio_tag_print(wtk_audio_tag_t *tag);
#ifdef __cplusplus
};
#endif
#endif
