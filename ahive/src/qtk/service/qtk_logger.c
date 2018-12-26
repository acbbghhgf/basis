#include "qtk/zeus/qtk_zeus.h"
#include "wtk/core/wtk_alloc.h"
#include "qtk/zeus/qtk_zeus_server.h"
#include <errno.h>

typedef struct qtk_logger qtk_logger_t;

struct qtk_logger {
    FILE *handle;
    char *filename;
    time_t cached_time;
    char log_time[32];
    int close;
};

qtk_logger_t* logger_create(void) {
    qtk_logger_t *inst = wtk_malloc(sizeof(*inst));
    inst->cached_time = time(NULL);
    inst->handle = NULL;
    inst->filename = NULL;
    inst->close = 0;
    memset(inst->log_time, 0, sizeof(inst->log_time));
    return inst;
}

void logger_release(qtk_logger_t *logger) {
    if (logger->close) {
        fclose(logger->handle);
    }
    if (logger->filename) {
        wtk_free(logger->filename);
    }
    wtk_free(logger);
}


static int logger_update_time(qtk_logger_t *logger) {
    time_t ct;
    struct tm xm;
    int ret = -1;

    if (!time(&ct)) {
        const char *es = strerror(errno);
        wtk_debug("time failed: %s\r\n", es);
        qtk_zserver_log("time failed: %s", es);
        goto end;
    }
    if (ct != logger->cached_time) {
        logger->cached_time = ct;
    } else {
        ret = 0;
        goto end;
    }
    if (!localtime_r(&ct, &xm)) {
        const char *es = strerror(errno);
        wtk_debug("localtime_r failed: %s\r\n", es);
        qtk_zserver_log("localtime_r failed: %s", es);
        goto end;
    }
    sprintf(logger->log_time,"%04d-%02d-%02d %02d:%02d:%02d",
            xm.tm_year+1900, xm.tm_mon+1, xm.tm_mday, xm.tm_hour, xm.tm_min, xm.tm_sec);
    ret = 0;
end:
    return ret;
}

static int logger_cb(struct qtk_zcontext *ctx, void *ud, int type, int session, uint32_t src, const char *data, size_t sz) {
    qtk_logger_t *inst = ud;
    switch (type) {
    case ZEUS_PTYPE_SYSTEM:
        if (inst->filename) {
            inst->handle = freopen(inst->filename, "a", inst->handle);
        }
        break;
    case ZEUS_PTYPE_TEXT:
        logger_update_time(inst);
        fprintf(inst->handle, "%s [:%08x] %.*s\n", inst->log_time, src, (int)sz, data);
        fflush(inst->handle);
        break;
    }

    return 0;
}


int logger_init(qtk_logger_t *inst, struct qtk_zcontext *ctx, const char *param) {
    if (param) {
        inst->handle = fopen(param, "w");
        if (NULL == inst->handle) {
            return -1;
        }
        int name_size = strlen(param);
        inst->filename = wtk_malloc(name_size+1);
        memcpy(inst->filename, param, name_size);
        inst->filename[name_size] = '\0';
        inst->close = 1;
    } else {
        inst->handle = stdout;
    }
    if (inst->handle) {
        qtk_zcallback(ctx, inst, logger_cb);
        qtk_zcommand(ctx, "REG", ".logger");
        return 0;
    }
    return -1;
}
