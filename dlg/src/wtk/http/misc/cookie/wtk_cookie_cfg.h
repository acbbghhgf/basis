#ifndef WTK_HTTP_MISC_COOKIE_WTK_COOKIE_CFG_H_
#define WTK_HTTP_MISC_COOKIE_WTK_COOKIE_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_cookie_cfg wtk_cookie_cfg_t;

/*
 * @brief use for set-cookie handler;
 */

struct wtk_cookie_cfg
{
	wtk_local_cfg_t *lc;
	wtk_str_hash_t *hash;
	wtk_strbuf_t *cookie;
	wtk_string_t str;
	unsigned update_cookie:1;
};

int wtk_cookie_cfg_init(wtk_cookie_cfg_t *cfg);
int wtk_cookie_cfg_clean(wtk_cookie_cfg_t *cfg);
int wtk_cookie_cfg_update_local(wtk_cookie_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_cookie_cfg_update(wtk_cookie_cfg_t *cfg);
void wtk_cookie_cfg_update_cookie(wtk_cookie_cfg_t *cfg,char *data,int len);
#ifdef __cplusplus
};
#endif
#endif
