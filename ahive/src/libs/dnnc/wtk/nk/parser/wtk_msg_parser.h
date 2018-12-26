#ifndef WTK_NK_PARSER_WTK_MSG_PARSER
#define WTK_NK_PARSER_WTK_MSG_PARSER
#include "wtk/core/wtk_type.h" 
#include "wtk/nk/wtk_connection.h"
#include "wtk/core/wtk_strbuf.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_msg_parser wtk_msg_parser_t;
typedef enum
{
	WTK_MSG_PARSER_INIT=0,
	WTK_MSG_PARSER_HDR,
	WTK_MSG_PARSER_DATA,
}wtk_msg_parser_state_t;

typedef void(*wtk_msg_parser_raise_f)(void *ths,wtk_msg_parser_t *parser,char *data,int len);

/*
 * protocol:
 *   1) 4 byte, sizeof msg;
 *   2) msg
 */
struct wtk_msg_parser
{
	wtk_connection_t *c;
	wtk_strbuf_t *buf;
	wtk_msg_parser_state_t state;
	union
	{
		uint32_t v;
		char data[4];
	}msg_size;
	int msg_size_pos;
	void *hook;
	void *ths;
	wtk_msg_parser_raise_f raise;
};

wtk_msg_parser_t* wtk_msg_parser_new(wtk_connection_t *c);
void wtk_msg_parser_delete(wtk_msg_parser_t *p);
void wtk_msg_parser_reset(wtk_msg_parser_t *p);
void wtk_msg_parser_set_raise(wtk_msg_parser_t *p,void *ths,wtk_msg_parser_raise_f raise);
int wtk_msg_parser_feed(wtk_msg_parser_t *p,char *data,int len);
#ifdef __cplusplus
};
#endif
#endif
