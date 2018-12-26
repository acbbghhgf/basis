#include "wtk_log.h"
#include <stdarg.h>
#include <time.h>
#include "wtk/core/wtk_os.h"
wtk_log_t *glb_log=NULL;


int wtk_log_g_print_time(char *buf)
{
	time_t ct;
	struct tm xm;
	struct tm* m;
	int ret,n;

	n=0;
	ret=time(&ct);
	if(ret==-1){goto end;}
#ifdef WIN32
	m=localtime(&ct);
#else
	m=localtime_r(&ct,&xm);
#endif
	if(!m){ret=-1;goto end;}
	n=sprintf(buf,"%04d-%02d-%02d-%02d:%02d:%02d",m->tm_year+1900,m->tm_mon+1,m->tm_mday,m->tm_hour,m->tm_min,m->tm_sec);
	ret=0;
end:
	return n;
}

wtk_log_t* wtk_log_new(char *fn)
{
	return wtk_log_new2(fn,LOG_NOTICE);
}

wtk_log_t* wtk_log_new2(char *fn,int level)
{
	return wtk_log_new3(fn,level,0);
}

wtk_log_t* wtk_log_new3(char *fn,int level, int daily)
{
	wtk_log_t* l;
	int ret;

	l=(wtk_log_t*)wtk_malloc(sizeof(*l));
	l->log_level=level;
	l->log_ts=1;
    l->daily=daily;
    l->log_touch=1;
	ret=wtk_log_init(l,fn);
	if(ret!=0)
	{
		wtk_log_delete(l);
		l=0;
	}
	glb_log=l;
	return l;
}


int wtk_log_delete(wtk_log_t *l)
{
	wtk_log_clean(l);
	wtk_free(l);
	return 0;
}

/* get log file */
static FILE * 
_f(wtk_log_t *l)
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
            l->f = wtk_file_open(fn, "w");
            l->today = l->t->tm.tm_mday;
        }
    } else {
        if (l->f == NULL) {
            snprintf(fn, sizeof(fn), "%s", l->fn);
            l->f = wtk_file_open(fn, "w");
        }
    }

    return l->f;
}


int wtk_log_init(wtk_log_t* l,char* fn)
{
	l->t = wtk_time_new();
    l->today = -1;
    l->f = NULL;
    l->fn[0] = 0;

    if (fn) {
        strncpy(l->fn, fn, sizeof(l->fn));
    }

    l->f = _f(l);

	wtk_lock_init(&(l->l));
	return l->f ? 0 : -1;
}

int wtk_log_clean(wtk_log_t* l)
{
	int ret;

	wtk_lock_clean(&(l->l));
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

int wtk_log_redirect(wtk_log_t *l,char *fn)
{
	fclose(l->f);
	l->f=wtk_file_open(fn,"a");
	return l->f?0:-1;
}

int wtk_log_printf(wtk_log_t* l,const char *func,int line,int level,char* fmt,...)
{
	va_list ap;
	int ret;

	if(level<l->log_level)
	{
		return 0;
	}
    //if(!l){return 0;}
	va_start(ap,fmt);
	ret=wtk_lock_lock(&(l->l));
	if(ret!=0){goto end;}
    if (l->f == NULL) {
        goto end;
    }
    if(l->log_touch)
    {
    	wtk_time_update(l->t);
    }
    l->f = _f(l);
	//fprintf(l->f,"%*.*s:%f [%d](%s:%d) ",l->t->log_time.len,l->t->log_time.len,l->t->log_time.data,time_get_ms(),level,func,line);
	if(l->log_ts)
	{
		//wtk_time_update(l->t);
		fprintf(l->f,"%*.*s [%d](%s:%d) ",l->t->log_time.len,l->t->log_time.len,l->t->log_time.data,level,func,line);
	}
	vfprintf(l->f,fmt,ap);
	fprintf(l->f,"\n");
	fflush(l->f);
	//wtk_debug("ret=%d\n",ret);
	ret=wtk_lock_unlock(&(l->l));
end:
	va_end(ap);
	return 0;
}

double wtk_log_cur_time(wtk_log_t *l)
{
	return l->t->wtk_cached_time;
}



//--------------------- test/example section ----------------
void wtk_log_test_g()
{
	wtk_log_t *log;
	int i=0;

	log=wtk_log_new("foo.log");
	while(1)
	{
		wtk_log_log(log,"log %d.",++i);
		wtk_msleep(500);
	}
	wtk_log_delete(log);
}
