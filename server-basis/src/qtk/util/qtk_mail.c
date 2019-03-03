#include "qtk_mail.h"
#include "wtk/os/wtk_lock.h"
#include <sys/wait.h>

static wtk_strbuf_t *mail_buf = NULL;
static int pid;

int qtk_mail_init(wtk_strbuf_t *tmpl) {
    mail_buf= wtk_strbuf_new(1024 * 10, 1);
    wtk_strbuf_push(mail_buf, tmpl->data, tmpl->pos);
    return 0;
}


int qtk_mail_send(const char *msg, int len) {
    int old_pos = mail_buf->pos;
    int ret = 0;
    if ((pid = vfork()) == 0) { /* process 1 */
        pid = vfork();
        if (pid == 0) {
            /* process 3 */
            wtk_strbuf_push(mail_buf, msg, len);
            wtk_strbuf_push_s0(mail_buf, "'''\n\nsend_alert_mail(content)");
            mail_buf->pos = old_pos;
            execlp("timeout", "timeout", "-s", "SIGKILL", "15", "python", "-c", mail_buf->data, NULL);
        } else if (pid > 0) {
            /* process 2, exit then init process will wait 3 */
            exit(0);
        }
    }
    waitpid(pid, NULL, 0);
    return ret;
}


int qtk_mail_clean() {
    wtk_strbuf_delete(mail_buf);
    mail_buf = NULL;
    return 0;
}
