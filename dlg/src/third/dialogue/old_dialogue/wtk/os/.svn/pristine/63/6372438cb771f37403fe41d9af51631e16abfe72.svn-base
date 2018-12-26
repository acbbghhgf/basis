#include "wtk_time.h"
#include "wtk/core/wtk_str.h"
#include <ctype.h>
#include <time.h>

static char* month[]={
		"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

static char* wkday[]={
		"Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};

wtk_time_t* wtk_time_new()
{
	wtk_time_t *t;
	int ret;

	t=(wtk_time_t*)wtk_malloc(sizeof(*t));
	ret=wtk_time_init(t);
	if(ret!=0)
	{
		wtk_time_delete(t);
		t=0;
	}
	return t;
}

int wtk_time_delete(wtk_time_t *t)
{
	wtk_time_clean(t);
	wtk_free(t);
	return 0;
}

int wtk_time_init(wtk_time_t* t)
{
	int ret;

	ret=wtk_lock_init(&(t->lock));
	if(ret!=0){goto end;}
	t->slot=0;
	wtk_string_set(&(t->http_time),0,0);
	wtk_string_set(&(t->log_time),0,0);
	ret=wtk_time_update(t);
end:
	return ret;
}

int wtk_time_clean(wtk_time_t* t)
{
	return wtk_lock_clean(&(t->lock));
}

#ifdef WIN32
int wtk_time_update_direct(wtk_time_t* t)
{
    time_t ct;
    struct tm* m;
    char *p;
    int ret,n;

    t->wtk_cached_time=time_get_ms();
    ret=time(&ct);
    if(ret==-1)
    {
        perror("time failed.");
        goto end;
    }
    m=gmtime(&ct);
    if(!m)
    {
        perror("gmtime failed.");
        goto end;
    }
    ++t->slot;
    if(t->slot>=WTK_TIME_SLOTS)
    {
        t->slot=0;
    }
    ret=0;
    p=t->cached_http_time[t->slot];
    n=sprintf(p,"%s, %d %s %04d %02d:%02d:%02d GMT",wkday[m->tm_wday],m->tm_mday,
     month[m->tm_mon],m->tm_year+1900,m->tm_hour,m->tm_min,m->tm_sec);
    t->http_time.data=p;
    t->http_time.len=n;
    p=t->cached_log_time[t->slot];
    n=sprintf(p,"%04d/%02d/%02d %02d:%02d:%02d",m->tm_year+1900,m->tm_mon+1,m->tm_mday,m->tm_hour,m->tm_min,m->tm_sec);
    t->log_time.data=p;
    t->log_time.len=n;
end:
    return ret;
}

#else

int wtk_time_update_direct(wtk_time_t* t)
{
	struct timeval   tv;
	time_t ct;
	struct tm xm;
	struct tm* m;
	char *p;
	int ret,n;

	ret=gettimeofday(&tv,0);
	if(ret==0)
	{
		//t->wtk_cached_time=tv.tv_sec*1000.0+tv.tv_usec*1.0/1000;
		t->wtk_cached_time=tv.tv_sec*1000.0+tv.tv_usec*0.001;//1.0/1000;
	}else
	{
		perror("gettimeofday failed.");
	}
	//wtk_debug("update: %f\n",t->wtk_cached_time);
	ret=time(&ct);
	if(ret==-1)
	{
		perror("time failed.");
		goto end;
	}
	//m=gmtime(&ct);
	m=localtime_r(&ct,&xm);
	if(!m)
	{
		perror("gmtime failed.");
		goto end;
	}
	++t->slot;
	if(t->slot>=WTK_TIME_SLOTS)
	{
		t->slot=0;
	}
	t->tm=*m;
	ret=0;
	p=t->cached_http_time[t->slot];
	n=sprintf(p,"%s, %d %s %04d %02d:%02d:%02d GMT",wkday[m->tm_wday],m->tm_mday,
			month[m->tm_mon],m->tm_year+1900,m->tm_hour,m->tm_min,m->tm_sec);
	t->http_time.data=p;
	t->http_time.len=n;
	p=t->cached_log_time[t->slot];
	n=sprintf(p,"%04d-%02d-%02d %02d:%02d:%02d",m->tm_year+1900,m->tm_mon+1,m->tm_mday,m->tm_hour,m->tm_min,m->tm_sec);
	t->log_time.data=p;
	t->log_time.len=n;
end:
	return ret;
}
#endif

int wtk_time_update(wtk_time_t* t)
{
    int ret;

    ret=wtk_lock_lock(&t->lock);
    if(ret!=0)
    {
        perror(__FUNCTION__);
        goto end;
    }
    ret=wtk_time_update_direct(t);
    if(ret!=0)
    {
        perror(__FUNCTION__);
    }
    wtk_lock_unlock(&(t->lock));
end:
    return 0;
}

char* wtk_time_g_now()
{
	time_t t;

	t=time(0);
	return ctime(&t);
}
