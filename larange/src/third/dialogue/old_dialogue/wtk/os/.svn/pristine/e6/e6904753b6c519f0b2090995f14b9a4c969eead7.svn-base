#define __USE_GNU
#include "wtk_proc.h"
#ifdef WIN32
#include <Windows.h>
#else
#ifndef USE_R16
#include <execinfo.h>
#endif
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#endif
#include "wtk/core/wtk_os.h"

#ifdef WIN32
int proc_get_abspath(char* buf,int len)
{
    return GetModuleFileNameA(NULL,buf,len);
}
#else
int wtk_get_host_free_mem(double *v)
{
    struct sysinfo info;
    int ret;

    ret=sysinfo(&info);
    if(ret==0)
    {
        *v=(info.mem_unit*info.freeram)/(1024*1024);
    }else
    {
    	*v=-1;
    }
    return ret;
}

double wtk_proc_mem1()
{
	struct rusage ru;

	getrusage(RUSAGE_SELF,&ru);
	return ru.ru_maxrss*1.0/1024;
}

double wtk_proc_mem()
{
	wtk_statm_t m;
	double x=-1;
	int ret;

	ret=wtk_statm_init(&m);
	if(ret!=0){goto end;}
	x=m.resident*4.0/1024;
	//x=m.size*4.0/1024;
end:
	return x;
}

void wtk_statm_print(wtk_statm_t *m)
{
	float rate;

	rate=4.0/1024;
	//wtk_debug("============= statm ===============\n");
	printf("size: %.3fM\n",m->size*rate);
	printf("resident: %.3fM\n",m->resident*rate);
	printf("share: %.3fM\n",m->share*rate);
	printf("text: %.3fM\n",m->text*rate);
	printf("lib: %.3fM\n",m->lib*rate);
	printf("data: %.3fM\n",m->data*rate);
	printf("dt: %.3fM\n",m->dt*rate);
	fflush(stdout);
}

void wtk_print_statm()
{
	wtk_statm_t m;

	wtk_statm_init(&m);
	wtk_statm_print(&m);
}

int wtk_statm_init(wtk_statm_t *m)
{
	char buf[256];
	FILE *f;
	int ret=-1;

	/*
	//116551 98709 520 653 0 114831 0
	 * 98033 54203 664 808 0 90780 0
                  size       total program size
                             (same as VmSize in /proc/[pid]/status)
                  resident   resident set size
                             (same as VmRSS in /proc/[pid]/status)
                  share      shared pages (from shared mappings)
                  text       text (code)
                  lib        library (unused in Linux 2.6)
                  data       data + stack
                  dt         dirty pages (unused in Linux 2.6)
	 */
	sprintf(buf,"/proc/%d/statm",getpid());
	f=fopen(buf,"r");
	if(!f){goto end;}
	ret=fscanf(f,"%d %d %d %d %d %d %d",&(m->size),&(m->resident),&(m->share),&(m->text),&(m->lib),&(m->data),&(m->dt));
	if(ret!=7){ret=-1;goto end;}
	ret=0;
end:
	if(f)
	{
		fclose(f);
	}
	return ret;
}

void wtk_debug_mem()
{
	struct rusage ru;
	double v;

	wtk_debug("================ mem(%d) ==============\n",getpid());
	wtk_get_host_free_mem(&v);
	getrusage(RUSAGE_SELF,&ru);
	printf("self: %.3fM, free=%.3fM\n",ru.ru_maxrss*1.0/1024,v);
	getrusage(RUSAGE_CHILDREN,&ru);
	printf("child: %.3fM\n",ru.ru_maxrss*1.0/1024);
	getrusage(1,&ru);
	printf("thread: %.3fM\n",ru.ru_maxrss*1.0/1024);
	wtk_print_statm();
	fflush(stdout);
}


int proc_get_abspath(char* buf,int len)
{
    char exe[64];
    pid_t pid;
    ssize_t ret;

    pid=getpid();
    sprintf(exe,"/proc/%d/exe",pid);
    ret=readlink(exe,buf,len);
    if(ret>0 && ret<len-1)
    {
        buf[ret]=0;
        return ret;
    }else
    {
        return -1;
    }
}

#endif

int wtk_proc_get_dir(char* buf,int len)
{
#ifdef WIN32
#define DIR_SEP '\\'
#else
#define DIR_SEP '/'
#endif
    int i,ret;

    ret=proc_get_abspath(buf,len);
    if(ret>0)
    {
        for(i=ret-1;i>=0;--i)
        {
            if(buf[i]==DIR_SEP)
            {
                buf[i]=0;
                ret=i;
                break;
            }
        }
    }
    return ret;
}

#ifdef WIN32
void wtk_proc_msleep(int ms)
{
    Sleep(ms);
}
#else
void wtk_proc_dump_stack()
{
#ifndef USE_R16
    void* array[100];
    int size,i;
    char** strings;
    //int fd;
    //char buf[64];

    //wtk_debug_mem();
    size=backtrace(array,sizeof(array)/sizeof(void*));
    strings=backtrace_symbols(array,size);
    printf("################## %d stack frames ################\n",size);
    for(i=0;i<size;++i)
    {
        printf("%s\n",strings[i]);
    }
    free(strings);

    /*
    sprintf(buf,"%.0f_%d.log",time_get_ms(),getpid());
    printf("############### save to %s #######################\n",buf);
    fd=open(buf,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
    backtrace_symbols_fd(array,size,fd);
    close(fd);
    */
#endif
}

void wtk_proc_dump_stack2()
{
#ifndef USE_R16
    void* array[100];
    int size,i;
    char** strings;

    //wtk_debug_mem();
    size=backtrace(array,sizeof(array)/sizeof(void*));
    strings=backtrace_symbols(array,size);
    printf("################## %d stack frames ################\n",size);
    for(i=0;i<size;++i)
    {
        printf("%s\n",strings[i]);
    }
    free(strings);
#endif
}


void wtk_proc_msleep(int ms)
{
#ifdef WIN32
    Sleep(ms);
#else
    usleep(ms*1000);
#endif
}
#endif

