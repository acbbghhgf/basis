#ifndef _QTK_CONF_QTK_CONF_FILE_H
#define _QTK_CONF_QTK_CONF_FILE_H
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_stack.h"
#include "wtk/os/wtk_time.h"
#include "qtk/os/qtk_log.h"
#include "wtk/core/wtk_arg.h"
#include "qtk/frame/qtk_frame_cfg.h"
#include "wtk/core/cfg/wtk_cfg_file.h"
#include "wtk/core/wtk_arg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct qtk_conf_file qtk_conf_file_t;

struct qtk_conf_file
{
	qtk_fwd_cfg_t framework;
	wtk_cfg_file_t *cfg;
	qtk_log_t *log;
	wtk_string_t *lib_path;
	wtk_heap_t *heap;
};

qtk_conf_file_t* qtk_conf_file_new(char *lib_path,char *cfg,wtk_arg_t *arg);
int qtk_conf_file_delete(qtk_conf_file_t *c);
int qtk_conf_file_update(qtk_conf_file_t *c,wtk_arg_t *arg);
void qtk_conf_file_print(qtk_conf_file_t *c);
#ifdef __cplusplus
};
#endif
#endif
