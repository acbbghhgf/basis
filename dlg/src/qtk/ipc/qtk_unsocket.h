#ifndef _QTK_IPC_QTK_UNSOCKET_H_
#define _QTK_IPC_QTK_UNSOCKET_H_

#include <sys/un.h>
#include <glob.h>

typedef struct qtk_unsocket qtk_unsocket_t;

struct qtk_unsocket {
    int fd;
    struct sockaddr_un local;
    struct sockaddr_un remote;
    unsigned passive : 1;
    unsigned bind    : 1;
};

qtk_unsocket_t *qtk_unsocket_new();
qtk_unsocket_t *qtk_unsocket_bind(const char *p, size_t l);
qtk_unsocket_t *qtk_unsocket_serve(const char *p, size_t l, int backlog);
int qtk_unsocket_connect(qtk_unsocket_t *s, const char *p, size_t l);
int qtk_unsocket_init(qtk_unsocket_t *s);
void qtk_unsocket_clean(qtk_unsocket_t *s);
void qtk_unsocket_delete(qtk_unsocket_t *s);

#endif
