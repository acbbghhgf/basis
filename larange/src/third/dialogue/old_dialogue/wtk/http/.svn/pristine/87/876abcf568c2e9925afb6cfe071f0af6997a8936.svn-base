#ifndef WTK_HTTP_PROTO_WTK_REQUEST_CFG_H_
#define WTK_HTTP_PROTO_WTK_REQUEST_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/core/wtk_str_hash.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_request_cfg wtk_request_cfg_t;
struct wtk_request;
typedef int (*wtk_request_hdr_parse_f)(struct wtk_request *req,char *v,int len);

typedef struct
{
	wtk_string_t *from;
	wtk_string_t *to;
}wtk_request_fromto_t;

struct wtk_request_cfg
{
	wtk_array_t *redirect_custom_hdr;		//wtk_request_fromto_t*  array;
	wtk_str_hash_t *hdr_hash;
	int hdr_slot;
	int heap_size;
	int buf_size;
	float buf_rate;
	float body_buf_rate;
	unsigned use_hash:1;
	unsigned use_times:1;
};

int wtk_request_cfg_init(wtk_request_cfg_t *cfg);
int wtk_request_cfg_clean(wtk_request_cfg_t *cfg);
int wtk_request_cfg_update_local(wtk_request_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_request_cfg_update(wtk_request_cfg_t *cfg);
wtk_request_hdr_parse_f wtk_request_cfg_find_hdr_parse(wtk_request_cfg_t *cfg,char *k,int len);
#ifdef __cplusplus
};
#endif
#endif
