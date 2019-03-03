/*
 * libwebsockets-test-server - libwebsockets test implementation
 *
 * Copyright (C) 2011-2016 Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * The person who associated a work with this deed has dedicated
 * the work to the public domain by waiving all of his or her rights
 * to the work worldwide under copyright law, including all related
 * and neighboring rights, to the extent allowed by law. You can copy,
 * modify, distribute and perform the work, even for commercial purposes,
 * all without asking permission.
 *
 * The test apps are intended to be adapted for use in your code, which
 * may be proprietary.  So unlike the library itself, they are licensed
 * Public Domain.
 */
#include "../libwebsockets.h"
#include "test-server.h"
#include "../private-libwebsockets.h"
#include "qtk/event/qtk_event.h"

int close_testing;
int max_poll_elements;
int debug_level = 7;
volatile int force_exit = 0;
struct lws_context *context;
struct lws_plat_file_ops fops_plat;

#if defined(LWS_OPENSSL_SUPPORT) && defined(LWS_HAVE_SSL_CTX_set1_param)
char crl_path[1024] = "";
#endif

struct per_session_data__test {
    char buf[4096];
    int pos;
};

/* singlethreaded version --> no locks */

void test_server_lock(int care)
{
}
void test_server_unlock(int care)
{
}

/*
 * This demo server shows how to use libwebsockets for one or more
 * websocket protocols in the same server
 *
 * It defines the following websocket protocols:
 *
 *  dumb-increment-protocol:  once the socket is opened, an incrementing
 *				ascii string is sent down it every 50ms.
 *				If you send "reset\n" on the websocket, then
 *				the incrementing number is reset to 0.
 *
 *  lws-mirror-protocol: copies any received packet to every connection also
 *				using this protocol, including the sender
 */

enum demo_protocols {
    /* always first */
    PROTOCOL_HTTP = 0,

    PROTOCOL_DUMB_INCREMENT,
    PROTOCOL_LWS_MIRROR,
    PROTOCOL_LWS_STATUS,

    /* always last */
    DEMO_PROTOCOL_COUNT
};


int callback_echo(struct lws *wsi, enum lws_callback_reasons reason, void *user,
                  void *in, size_t len) {
    unsigned char buf[LWS_PRE + 4096];
    struct per_session_data__test *data = (struct per_session_data__test*)user;
    unsigned char *p = &buf[LWS_PRE];
    int m, n;

    switch (reason) {
    case LWS_CALLBACK_ESTABLISHED:
        data->pos = 0;
        break;

    case LWS_CALLBACK_SERVER_WRITEABLE:
        n = min(data->pos, sizeof(buf)-LWS_PRE);
        memcpy(p, data->buf, n);
        m = lws_write(wsi, p, n, LWS_WRITE_TEXT);
        if (m < 0) {
            wtk_debug("ERROR %d writing to di socket\n", data->pos);
            return -1;
        } else {
            data->pos -= m;
            memmove(data->buf, data->buf+m, data->pos);
        }
        break;

    case LWS_CALLBACK_RECEIVE:
        assert(len + data->pos <= sizeof(data->buf));
        memcpy(data->buf+data->pos, in, len);
        data->pos = data->pos + len;
        if (len == data->pos) {
            lws_callback_on_writable(wsi);
        }
        break;

    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
        dump_handshake_info(wsi);
        break;

    case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
        wtk_debug("LWS_CALLBACK_WS_PEER_INITIATED_CLOSE: len %lu\n",
                  (unsigned long)len);
        for (n = 0; n < (int)len; n++) {
            printf(" %d: 0x%02X\n", n, ((unsigned char *)in)[n]);
        }
        break;

    default:
        break;
    }
    return 0;
}


/* list of supported protocols and callbacks */

static struct lws_protocols protocols[] = {
    {
        "echo",		/* name */
        callback_echo,		/* callback */
        sizeof (struct per_session_data__test),	/* per_session_data_size */
        0,			/* max frame size / rx buffer */
    },
    { NULL, NULL, 0, 0 } /* terminator */
};


/* this shows how to override the lws file operations.  You don't need
 * to do any of this unless you have a reason (eg, want to serve
 * compressed files without decompressing the whole archive)
 */
static lws_fop_fd_t
test_server_fops_open(const struct lws_plat_file_ops *fops,
              const char *vfs_path, const char *vpath,
              lws_fop_flags_t *flags)
{
    lws_fop_fd_t n;

    /* call through to original platform implementation */
    n = fops_plat.open(fops, vfs_path, vpath, flags);

    lwsl_notice("%s: opening %s, ret %p\n", __func__, vfs_path, n);

    return n;
}


static int test_accept_cb(struct lws_context *context, qtk_event_t *ev) {
    qtk_event2_t *ev2 = lws_container_of(ev, qtk_event2_t, ev);
    struct lws_pollfd eventfd;

    if (ev2->ev.error)
        return -1;

    eventfd.fd = ev2->fd;
    eventfd.events = 0;
    eventfd.revents = 0;

    eventfd.events |= LWS_POLLIN;
    eventfd.revents |= LWS_POLLIN;

    lws_service_fd(context, &eventfd);
    return 0;
}


static int test_client_cb(struct lws_context *context, qtk_event_t *ev) {
    qtk_event2_t *ev2 = lws_container_of(ev, qtk_event2_t, ev);
    struct lws_pollfd eventfd;

    eventfd.fd = ev2->fd;
    eventfd.events = 0;
    eventfd.revents = 0;

    if (ev->read) {
        eventfd.events |= LWS_POLLIN;
        eventfd.revents |= LWS_POLLIN;
    }

    if (ev->write) {
        eventfd.events |= LWS_POLLOUT;
        eventfd.revents |= LWS_POLLOUT;
    }

    if (ev->eof || ev->reof || ev->error) {
        eventfd.events |= LWS_POLLHUP;
        eventfd.revents |= LWS_POLLHUP;
    }

    lws_service_fd(context, &eventfd);
    return 0;
}


static int test_epoll_init(struct lws_context *context, qtk_event_module_t *em, int tsi) {
    struct lws_vhost *vh = context->vhost_list;

    context->pt[tsi].io_loop_qev = em;

    while (vh) {
        int fd = vh->lserv_wsi->desc.sockfd;
        if (vh->lserv_wsi) {
            vh->lserv_wsi->event.fd = fd;
            qtk_event_init(&vh->lserv_wsi->event.ev, QTK_EVENT_READ,
                           (qtk_event_handler)test_accept_cb, NULL, context);
            qtk_event_add_event(em, fd, &vh->lserv_wsi->event.ev);
        }
        vh = vh->vhost_next;
    }

    return 0;
}


void lws_libqev_io(struct lws* wsi, int flags) {
    struct lws_context_per_thread *pt = &wsi->context->pt[(int)wsi->tsi];

    if (!pt->io_loop_qev) {
        return;
    }

    assert((flags & (LWS_EV_START| LWS_EV_STOP)) &&
           (flags & (LWS_EV_READ | LWS_EV_WRITE)));

    if (flags & LWS_EV_START) {
        if (flags & LWS_EV_WRITE) {
            wsi->event.ev.want_write = 1;
        }
        if (flags & LWS_EV_READ) {
            wsi->event.ev.want_read = 1;
        }
        qtk_event_mod_event(pt->io_loop_qev, wsi->event.fd, &wsi->event.ev);
    } else {
        if (flags & LWS_EV_WRITE) {
            wsi->event.ev.want_write = 0;
        }
        if (flags & LWS_EV_READ) {
            wsi->event.ev.want_read = 0;
        }
        if (wsi->event.ev.want_read || wsi->event.ev.want_write) {
            qtk_event_mod_event(pt->io_loop_qev, wsi->event.fd, &wsi->event.ev);
        } else {
            qtk_event_del_event(pt->io_loop_qev, wsi->event.fd);
        }
    }
}


int lws_libqev_init_fd_table(struct lws_context *context) {
    return 0;
}


void lws_libqev_accept(struct lws *new_wsi, lws_sock_file_fd_type desc) {
    struct lws_context *context = lws_get_context(new_wsi);
    qtk_event_t *ev = &new_wsi->event.ev;

    if (new_wsi->mode == LWSCM_RAW_FILEDESC)
        new_wsi->event.fd = desc.filefd;
    else
        new_wsi->event.fd = desc.sockfd;

    qtk_event_init(ev, 0,
                   (qtk_event_handler)test_client_cb,
                   (qtk_event_handler)test_client_cb, context);
}


int test_websocket(int argc, char **argv)
{
    struct lws_context_creation_info info;

    /*
     * take care to zero down the info struct, he contains random garbaage
     * from the stack otherwise
     */
    memset(&info, 0, sizeof info);
    info.port = 7681;

    info.iface = NULL;
    info.protocols = protocols;
    info.extensions = NULL;

    info.gid = -1;
    info.uid = -1;
    info.max_http_header_pool = 1;
    info.options = 0;

    context = lws_create_context(&info);
    if (context == NULL) {
        lwsl_err("libwebsocket init failed\n");
        return -1;
    }

    /*
     * this shows how to override the lws file operations.  You don't need
     * to do any of this unless you have a reason (eg, want to serve
     * compressed files without decompressing the whole archive)
     */
    /* stash original platform fops */
    fops_plat = *(lws_get_fops(context));
    /* override the active fops */
    lws_get_fops(context)->open = test_server_fops_open;

    qtk_event_cfg_t cfg;
    qtk_event_cfg_init(&cfg);
    qtk_event_cfg_update(&cfg);
    qtk_event_module_t *em = qtk_event_module_new(&cfg);

    test_epoll_init(context, em, 0);

    while (1) {
        qtk_event_module_process(em, 100, 0);
    }

    lws_context_destroy(context);
    qtk_event_module_delete(em);

    return 0;
}
