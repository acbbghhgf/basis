#ifndef QTK_ZEUS_QTK_ZEUS_SOCKET_H
#define QTK_ZEUS_QTK_ZEUS_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct qtk_zsocket_msg qtk_zsocket_msg_t;

#define ZSOCKET_MODE_SHIFT 30
#define ZSOCKET_MODE_BITS (sizeof(int)*8-ZSOCKET_MODE_SHIFT)

enum qtk_zsocket_open_mode {
    ZSOCKET_MODE_RDONLY = 0,
    ZSOCKET_MODE_WRONLY = 1,
    ZSOCKET_MODE_RDWR = 2,
};

enum qtk_zsocket_msg_type {
    ZSOCKET_MSG_OPEN = 1,
    ZSOCKET_MSG_CLOSE,
    ZSOCKET_MSG_DATA,
    ZSOCKET_MSG_ERR,
    ZSOCKET_MSG_ACCEPT,
};

enum qtk_zsocket_cmd_type {
    ZSOCKET_CMD_OPEN = 0,
    ZSOCKET_CMD_SEND,
    ZSOCKET_CMD_CLOSE,
    ZSOCKET_CMD_LISTEN,
};

struct qtk_zsocket_msg {
    int type;
    int id;
};

#ifdef __cplusplus
}
#endif

#endif
