#ifndef WTK_NK_WTK_LISTEN_H_
#define WTK_NK_WTK_LISTEN_H_
#include "wtk/core/wtk_type.h"
#ifdef WIN32
#include <WinSock2.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#endif
#include "wtk_listen_cfg.h"
//#include "wtk_connection.h"
#ifdef __cplusplus
extern "C" {
#endif
struct wtk_connection;
typedef struct wtk_listen wtk_listen_t;
typedef struct wtk_parser wtk_parser_t;
typedef int (*wtk_parse_handler)(void* data,struct wtk_connection *c,char *buf,int len);
typedef int (*wtk_parser_close_handler)(void* data);
typedef int (*wtk_parser_shot_handler)(void*  data);
typedef int (*wtk_parser_close_notify_handler)(void* data);

#define WTK_PARSER  \
 wtk_parse_handler handler; \
 wtk_parser_close_handler close; \
 wtk_parser_shot_handler shot; \
 wtk_parser_close_notify_handler close_notify;

struct wtk_parser
{
	WTK_PARSER
};

typedef void* (*wtk_pop_parser_handler)(void* user_data,struct wtk_connection *c);
typedef int (*wtk_push_parser_handler)(void* user_data,void* parser);

struct wtk_listen
{
	wtk_listen_cfg_t *cfg;
	int fd;
	struct sockaddr_in addr;
	int type;
	uint16_t port;
	int backlog;
	wtk_pop_parser_handler pop_parser;
	wtk_push_parser_handler push_parser;
	void*	user_data;
	struct wtk_connection *con;
};

/**
 * @brief initialize.
 */
int wtk_listen_init(wtk_listen_t* l,wtk_listen_cfg_t *cfg,short port,wtk_pop_parser_handler,wtk_push_parser_handler,void *user_data);
void wtk_listen_set_parser_handler(wtk_listen_t *l,void *data,wtk_pop_parser_handler pop,wtk_push_parser_handler push);

/**
 * @brief open file descriptor.
 */
int wtk_listen_listen(wtk_listen_t* l,int loop);
void wtk_listen_cpy(wtk_listen_t *dst,wtk_listen_t *src);
void wtk_listen_close_fd(wtk_listen_t *l);
#ifdef __cplusplus
};
#endif
#endif
