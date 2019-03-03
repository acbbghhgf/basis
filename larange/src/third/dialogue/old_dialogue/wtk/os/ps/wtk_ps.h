#ifndef WTK_OS_PS_WTK_PS_H_
#define WTK_OS_PS_WTK_PS_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_strmsg.h"
#include "wtk_ps_cfg.h"
#include "wtk_psys.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_ps wtk_ps_t;

struct wtk_ps
{
	wtk_ps_cfg_t *cfg;
	//wtk_strbuf_t *buf;
	wtk_psys_t sys;
	char *tmp;
};

wtk_ps_t* wtk_ps_new(wtk_ps_cfg_t *cfg);
void wtk_ps_delete(wtk_ps_t *p);

/**
 *	@brief end with wtk_string_t *str and NULL
 */
int wtk_ps_process(wtk_ps_t *p,wtk_strbuf_t *buf,char *cmd,...);

//-------------------------------------------------------------
int wtk_ps_start(wtk_ps_t *p,char *cmd);
void wtk_ps_stop(wtk_ps_t *p);
int wtk_ps_restart(wtk_ps_t *p);
int wtk_ps_write_arg(wtk_ps_t *p,wtk_string_t *arg);
int wtk_ps_write_arg_line(wtk_ps_t *p,wtk_string_t *arg);
int wtk_ps_read_result(wtk_ps_t *p,wtk_strbuf_t *buf);

/**
 * @brief read block msg result;
 */
int wtk_ps_read_result3(wtk_ps_t *p,wtk_strbuf_t *buf);

/**
 * @brief read non-block result
 */
int wtk_ps_read_result4(wtk_ps_t *p,wtk_strbuf_t *buf,int seek);

#ifdef __cplusplus
};
#endif
#endif
