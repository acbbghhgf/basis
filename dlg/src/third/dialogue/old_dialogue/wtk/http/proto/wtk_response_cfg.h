#ifndef WTK_HTTP_PROTO_WTK_RESPONSE_CFG_H_
#define WTK_HTTP_PROTO_WTK_RESPONSE_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_response_cfg wtk_response_cfg_t;
struct wtk_response_cfg
{
	wtk_string_t server;
	unsigned use_server:1;
	unsigned use_times:1;
	unsigned use_stream_length:1;
	unsigned use_hook:1;
	unsigned use_log:1;
	unsigned use_stream_id:1;
	unsigned use_upserver:1;
	unsigned use_audio_url:1;
	unsigned use_test:1;
	unsigned use_custom_hdr:1;
	unsigned use_lua:1;
	unsigned use_stream_message:1;
	unsigned use_stream_mode:1;
	unsigned use_date:1;
	unsigned use_connection:1;
    unsigned use_connection_keep_alive_http1_1:1;
    unsigned use_content_type:1;
    unsigned use_content_type_text_plain:1;
};

int wtk_response_cfg_init(wtk_response_cfg_t *cfg);
int wtk_response_cfg_clean(wtk_response_cfg_t *cfg);
int wtk_response_cfg_update_local(wtk_response_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_response_cfg_update(wtk_response_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
