#include "qtk_mail.h"
#include "wtk/os/wtk_lock.h"
#include <sys/wait.h>

static wtk_strbuf_t *mail_buf = NULL;
static wtk_lock_t lock;
static int pid;

int qtk_mail_init(wtk_strbuf_t *tmpl) {
    mail_buf= wtk_strbuf_new(1024 * 10, 1);
    wtk_strbuf_push(mail_buf, tmpl->data, tmpl->pos);
    wtk_lock_init(&lock);
    return 0;
}


static void qtk_mail_kill(int sig) {
    if (SIGALRM == sig) {
        kill(pid, SIGKILL);
    }
}


int qtk_mail_send(const char *msg, int len) {
    int old_pos = mail_buf->pos;
    int ret = 0;
    wtk_lock_lock(&lock);
    wtk_strbuf_push(mail_buf, msg, len);
    wtk_strbuf_push_s0(mail_buf, "'''\n\nsend_alert_mail(content)");
    if ((pid = vfork()) == 0) {
        if (vfork() == 0) {
            pid = vfork();
            if (pid == 0) {
                execlp("python", "python", "-c", mail_buf->data, NULL);
            } else if (pid > 0) {
                signal(SIGALRM, qtk_mail_kill);
                waitpid(pid, NULL, 0);
            }
            exit(0);
        } else {
            exit(0);
        }
    }
    waitpid(pid, NULL, 0);
    mail_buf->pos = old_pos;
    wtk_lock_unlock(&lock);
    return ret;
}


int qtk_mail_clean() {
    wtk_strbuf_delete(mail_buf);
    wtk_lock_clean(&lock);
    mail_buf = NULL;
    return 0;
}
