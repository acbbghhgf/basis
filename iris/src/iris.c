#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include "iris.h"
#include "qtk_iris_cfg.h"
#include "qtk/core/cJSON.h"
#include "qtk/sframe/qtk_sframe.h"
#include "qtk/os/qtk_log.h"


typedef struct iris_conf_file iris_conf_file_t;

struct iris_conf_file {
    qtk_iris_cfg_t iris;
    qtk_log_t *log;
};


static int conf_file_delete(iris_conf_file_t* c) {
    qtk_iris_cfg_clean(&c->iris);
    if (c->log) {
        qtk_log_delete(c->log);
    }
    wtk_free(c);
    return 0;
}


static iris_conf_file_t* conf_file_new(wtk_local_cfg_t *lc, qtk_log_t *log) {
    iris_conf_file_t *conf;
    int ret = -1;

    conf = (iris_conf_file_t*)wtk_calloc(1, sizeof(*conf));
    qtk_iris_cfg_init(&conf->iris);
    ret = qtk_iris_cfg_update_local(&conf->iris, lc);
    if (ret) {
        wtk_debug("update iris local configure failed.\r\n");
        qtk_log_log0(log, "update iris local configure failed.");
        goto end;
    }
    ret = qtk_iris_cfg_update(&conf->iris);
    if (ret) {
        wtk_debug("update iris local dependents configure failed.\r\n");
        qtk_log_log0(log, "update iris local dependents configure failed.");
        goto end;
    }
    if (conf->iris.log_fn) {
        conf->log = qtk_log_new(conf->iris.log_fn);
    } else {
        conf->log = log;
    }
    ret = conf->log ? 0 : -1;
end:
    if (ret != 0 && conf) {
        conf_file_delete(conf);
        conf = NULL;
    }
    return conf;
}


qtk_iris_t* iris_new(void *s)
{
    qtk_iris_t  *ip;
    qtk_sframe_method_t *method = s;
    wtk_local_cfg_t *lc = method->get_cfg(s);
    qtk_log_t *log = method->get_log(s);
    iris_conf_file_t *conf = conf_file_new(lc, log);
    assert(conf);
    signal(SIGPIPE, SIG_IGN);
    if(!(ip = calloc(1, sizeof(*ip))))
        goto end;
    ip->director = qtk_director_new(&conf->iris.director, conf->log);
    ip->director->iris = ip;
    ip->s = s;
    ip->conf = conf;
    return ip;
end:
    if(ip)
        iris_delete(ip);
    return NULL;
}

int
iris_delete(void *data)
{
    qtk_iris_t  *ip;

    if(!(ip = data))
        return -1;
    if (ip->director) {
        qtk_director_delete(ip->director);
    }
    if (ip->conf) {
        conf_file_delete(ip->conf);
    }
    free(ip);
    return 0;
}

int
iris_stop(void *data)
{
    qtk_iris_t  *iris;

    if(!(iris = data))
        return -1;
    iris->run = 0;
    qtk_director_stop(iris->director);
    qtk_director_join(iris->director);

    return 0;
}

int
iris_start(void *data)
{
    qtk_iris_t  *iris;
    qtk_iris_cfg_t *cfg;
    qtk_sframe_method_t *method;
    qtk_sframe_msg_t *msg;
    qtk_sframe_listen_param_t *lis_param;
    int id;

    if(!(iris = data))
        return -1;
    iris->run = 1;
    cfg = &((iris_conf_file_t*)iris->conf)->iris;
    method = iris->s;
    qtk_director_set_method(iris->director, iris->s);
    qtk_director_start(iris->director);

    id = method->socket(iris->s);
    msg = method->new(iris->s, QTK_SFRAME_MSG_CMD, id);
    lis_param = LISTEN_PARAM_NEW(cfg->listen, cfg->port, cfg->backlog);
    method->set_cmd(msg, QTK_SFRAME_CMD_LISTEN, lis_param);
    method->push(iris->s, msg);

    return 0;
}
