#ifndef WTK_OS_TCP_WTK_TCP_MSG
#define WTK_OS_TCP_WTK_TCP_MSG
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_tcp_msg wtk_tcp_msg_t;

#define wtk_tcp_msg_set_s(msg,cmd,data,data_bytes) wtk_tcp_msg_set(msg,cmd,sizeof(cmd)-1,data,data_bytes)
#define wtk_tcp_msg_set_cmd_s(msg,cmd) wtk_tcp_msg_set_cmd(msg,cmd,sizeof(cmd)-1)

struct wtk_tcp_msg
{
	wtk_strbuf_t *cmd;
	wtk_strbuf_t *data;
};


wtk_tcp_msg_t* wtk_tcp_msg_new(int cmd_bytes,int data_bytes);
void wtk_tcp_msg_delete(wtk_tcp_msg_t *msg);
void wtk_tcp_msg_reset(wtk_tcp_msg_t *msg);
void wtk_tcp_msg_print(wtk_tcp_msg_t *msg);
int wtk_tcp_msg_read(wtk_tcp_msg_t *msg,int fd);
int wtk_tcp_msg_write(wtk_tcp_msg_t *msg,int fd);
void wtk_tcp_msg_set(wtk_tcp_msg_t *msg,char *cmd,int cmd_bytes,char* data,int data_bytes);
void wtk_tcp_msg_set_cmd(wtk_tcp_msg_t *msg,char *cmd,int cmd_bytes);

#ifdef __cplusplus
};
#endif
#endif
