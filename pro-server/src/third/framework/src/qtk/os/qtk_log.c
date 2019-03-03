#include "qtk_log.h"
#include <stdarg.h>
#include <time.h>
#include "wtk/core/wtk_os.h"
#include "qtk/util/qtk_mail.h"
#include <errno.h>
qtk_log_t *glb_qlog=NULL;


int qtk_log_g_print_time(char *buf)
{
    time_t ct;
    struct tm xm;
    struct tm* m;
    int ret,n;

    n=0;
    ret=time(&ct);
    if(ret==-1){goto end;}
    m=localtime_r(&ct,&xm);
    if(!m){ret=-1;goto end;}
    n=sprintf(buf,"%04d-%02d-%02d-%02d:%02d:%02d",m->tm_year+1900,m->tm_mon+1,m->tm_mday,m->tm_hour,m->tm_min,m->tm_sec);
    ret=0;
end:
    return n;
}

qtk_log_t* qtk_log_new(char *fn)
{
    return qtk_log_new2(fn,LOG_NOTICE);
}

qtk_log_t* qtk_log_new2(char *fn,int level)
{
    return qtk_log_new3(fn,level,0);
}

qtk_log_t* qtk_log_new3(char *fn,int level, int daily)
{
    qtk_log_t* l;
    int ret;

    l=(qtk_log_t*)wtk_malloc(sizeof(*l));
    l->log_level=level;
    l->log_ts=1;
    l->daily=daily;
    l->log_touch=1;
    ret=qtk_log_init(l,fn);
    if(ret!=0)
    {
        qtk_log_delete(l);
        l=0;
    }
    glb_qlog=l;
    return l;
}


int qtk_log_delete(qtk_log_t *l)
{
    qtk_log_clean(l);
    wtk_free(l);
    return 0;
}

/* get log file */
static FILE *
_f(qtk_log_t *l)
{
    char fn[256];

    if (l->f == stdout || l->fn[0] == 0) {
        return stdout;
    }

    if (l->daily) {
        if (l->today != l->t->tm.tm_mday) {
            snprintf(fn, sizeof(fn), "%s-%04d%02d%02d", l->fn,
                     l->t->tm.tm_year + 1900,
                     l->t->tm.tm_mon + 1,
                     l->t->tm.tm_mday);

            if (l->f) {
                fclose(l->f);
            }
            l->f = wtk_file_open(fn, "a");
            l->today = l->t->tm.tm_mday;
        }
    } else {
        if (l->f == NULL) {
            snprintf(fn, sizeof(fn), "%s", l->fn);
            l->f = wtk_file_open(fn, "a");
        }
    }
    if (NULL == l->f) {
        char mail_content[256];
        int n;
        n = snprintf(mail_content, sizeof(mail_content), "open log file \"%s\" failed: %s",
                     fn, strerror(errno));
        qtk_mail_send(mail_content, n);
    }

    return l->f;
}


int qtk_log_reopen(qtk_log_t *l) {
    pthread_spin_unlock(&l->l);
    wtk_time_delete(l->t);
    l->t = wtk_time_new();
    l->log_touch = 0;
    if (l->f) {
        fclose(l->f);
        l->f = NULL;
        l->f = _f(l);
    }
    return 0;
}


int qtk_log_init(qtk_log_t* l,char* fn)
{
    l->t = wtk_time_new();
    l->today = -1;
    l->f = NULL;
    l->fn[0] = 0;

    if (fn) {
        strncpy(l->fn, fn, sizeof(l->fn));
    }

    l->f = _f(l);

    pthread_spin_init(&(l->l), 0);
    return l->f ? 0 : -1;
}


int qtk_log_clean(qtk_log_t* l)
{
    int ret;

    pthread_spin_destroy(&(l->l));
    if(l->f && (l->f!=stdout))
    {
        ret=fclose(l->f);
        l->f=0;
    }else
    {
        ret=0;
    }
    if(l->t)
    {
        wtk_time_delete(l->t);
    }
    return ret;
}

int qtk_log_redirect(qtk_log_t *l,char *fn)
{
    fclose(l->f);
    l->f=wtk_file_open(fn,"a");
    return l->f?0:-1;
}

int qtk_log_printf(qtk_log_t* l,const char *func,int line,int level,char* fmt,...)
{
    va_list ap;
    int ret;

    if (level < l->log_level) {
        return 0;
    }
    va_start(ap,fmt);
    ret = pthread_spin_lock(&(l->l));
    if (ret != 0) {
        goto end;
    }
    if (l->f == NULL) {
        goto end;
    }
    if (l->log_touch) {
        wtk_time_update(l->t);
    }
    l->f = _f(l);
    if (NULL == l->f) {
        goto end;
    }
    if(l->log_ts) {
        //wtk_time_update(l->t);
        fprintf(l->f,"%*.*s [%d](%s:%d) ",l->t->log_time.len,l->t->log_time.len,l->t->log_time.data,level,func,line);
    }
    vfprintf(l->f,fmt,ap);
    fprintf(l->f,"\n");
    fflush(l->f);
end:
    ret=pthread_spin_unlock(&(l->l));
    va_end(ap);
    return 0;
}

double qtk_log_cur_time(qtk_log_t *l)
{
    return l->t->wtk_cached_time;
}
