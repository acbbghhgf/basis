#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#endif
#include <stdio.h>
#include "wtk_cpu.h"

wtk_string_t cpu_names[]={
		wtk_string("usr"),
		wtk_string("nice"),
		wtk_string("sys"),
		wtk_string("idl"),
		wtk_string("iowait"),
		wtk_string("irq"),
		wtk_string("softirq"),
		wtk_string("stead"),
		wtk_string("guest"),
};

wtk_string_t* wtk_cpu_g_get_name(wtk_cpu_index_t i)
{
	return &(cpu_names[i]);
}

int wtk_cpu_shot2(wtk_source_t *p,wtk_strbuf_t *buf,double *v)
{
	int ret;

	ret=wtk_source_read_string(p,buf);
	if(ret!=0){goto end;}
	ret=wtk_str_equal_s(buf->data,buf->pos,"cpu");
	if(ret!=1){ret=-1;goto end;}
	ret=wtk_source_read_double(p,v,WTK_CPU_SLOT);
	if(ret!=0){goto end;}
end:
	return ret;
}


int wtk_cpu_shot(wtk_source_t *p,wtk_strbuf_t *buf,double *v)
{
	FILE *file;
	int ret;
	//int i;

	file=(FILE*)p->data;
	if(!file){ret=-1;goto end;}
	rewind(file);
	fflush(file);
	ret=wtk_cpu_shot2(p,buf,v);
end:
	return ret;
}

wtk_cpu_t* wtk_cpu_new()
{
	wtk_cpu_t *cpu;
	int i,ret;

	cpu=(wtk_cpu_t*)wtk_malloc(sizeof(*cpu));
	cpu->index=0;
	cpu->ncpu=wtk_get_cpus();
	for(i=0;i<WTK_CPU_SLOT;++i)
	{
		//cpu->last_times[i]=0;
		cpu->cur_times[i]=0;
		cpu->diff[i]=0;
	}
	cpu->tot=0;
	cpu->buf=wtk_strbuf_new(32,1);
	ret=wtk_source_init_file2(&(cpu->src),"/proc/stat");
	if(ret!=0){goto end;}
	wtk_cpu_shot(&(cpu->src),cpu->buf,cpu->last_times);
end:
	if(ret!=0)
	{
		wtk_cpu_delete(cpu);
		cpu=0;
	}
	return cpu;
}

int wtk_cpu_delete(wtk_cpu_t *c)
{
	wtk_source_clean_file2(&(c->src));
	wtk_strbuf_delete(c->buf);
	wtk_free(c);
	return 0;
}

int wtk_cpu_update(wtk_cpu_t *c)
{
	int ret,i;

	++c->index;
	if(c->index>30000)
	{
		c->index=0;
	}
	ret=wtk_cpu_shot(&(c->src),c->buf,c->cur_times);
	if(ret!=0){goto end;}
	c->tot=0;
	for(i=0;i<WTK_CPU_SLOT;++i)
	{
		c->tot+=c->diff[i]=c->cur_times[i]-c->last_times[i];
		c->last_times[i]=c->cur_times[i];
	}
	for(i=0;i<WTK_CPU_SLOT;++i)
	{
		c->rate[i]=c->tot==0?0:c->diff[i]*100/c->tot;
	}
	if(c->tot==0)
	{
		c->rate[wtk_cpu_idle]=100.0;
	}
end:
	//wtk_cpu_print(c);
	return ret;
}

void wtk_cpu_to_string(wtk_cpu_t *cpu,wtk_strbuf_t *buf)
{
	int i;

	wtk_strbuf_push_s(buf,"{");
	for(i=wtk_cpu_user;i<=wtk_cpu_guest;++i)
	{
		if(i>wtk_cpu_user)
		{
			wtk_strbuf_push_s(buf,",");
		}
		wtk_strbuf_push_f(buf,"\"%*.*s\":%.1f",cpu_names[i].len,cpu_names[i].len,cpu_names[i].data,cpu->rate[i]);
	}
	wtk_strbuf_push_f(buf,",\"tot\":%.1f,\"index\":%d}",cpu->tot,cpu->index);
}

void wtk_cpu_to_string2(wtk_cpu_t *cpu,wtk_stack_t *stack)
{
	int i;

	wtk_stack_push_s(stack,"{");
	for(i=wtk_cpu_user;i<=wtk_cpu_guest;++i)
	{
		if(i>wtk_cpu_user)
		{
			wtk_stack_push_s(stack,",");
		}
		wtk_stack_push_f(stack,"\"%*.*s\":%.1f",cpu_names[i].len,cpu_names[i].len,cpu_names[i].data,cpu->rate[i]);
	}
	wtk_stack_push_f(stack,",\"tot\":%.1f,\"ncpu\":%d,\"index\":%d}",cpu->tot,cpu->ncpu,cpu->index);
}

void wtk_cpu_to_string3(wtk_cpu_t *cpu,wtk_stack_t *stack)
{
	int i;

	wtk_stack_push_s(stack,"{");
	for(i=wtk_cpu_user;i<=wtk_cpu_guest;++i)
	{
		wtk_stack_push_f(stack,"%*.*s=%.1f;",cpu_names[i].len,cpu_names[i].len,cpu_names[i].data,cpu->rate[i]);
	}
	wtk_stack_push_f(stack,"tot=%.1f;ncpu=%d;index=%d;}",cpu->tot,cpu->ncpu,cpu->index);
}

float wtk_cpu_rate(wtk_cpu_t *c)
{
	int ret;
	float r=0;

	ret=wtk_cpu_update(c);
	if(ret!=0 || c->tot<=0){goto end;}
	r=(c->diff[wtk_cpu_user]+c->diff[wtk_cpu_nice]+c->diff[wtk_cpu_system])/c->tot;
end:
	return r;
}

void wtk_cpu_print(wtk_cpu_t *c)
{
	wtk_string_t *v;
	int i;

	//f=time_get_ms();
	wtk_debug("============= %.1f ==============\n",time_get_ms());
	printf("rate: %.3f\n",(c->rate[wtk_cpu_user]+c->rate[wtk_cpu_nice]+c->rate[wtk_cpu_system]));
	for(i=0;i<WTK_CPU_SLOT;++i)
	{
		v=wtk_cpu_g_get_name(i);
		printf("%*.*s: %.3f\n",v->len,v->len,v->data,c->rate[i]);
	}
	//wtk_debug("================================\n");
}

#ifdef WIN32
int wtk_get_cpus()
{
    SYSTEM_INFO info;

   GetSystemInfo(&info);
   return info.dwNumberOfProcessors;
}

#else
int wtk_get_cpus()
{
	//return sysconf(_SC_NPROCESSORS_CONF);
    return sysconf(_SC_NPROCESSORS_ONLN);
}

/*
float wtk_cpu_get_frequence()
{
	float freq=-1;
	wtk_source_t src;
	wtk_strbuf_t *buf;
	int ret;

	ret=wtk_source_init_file(&(src),"/proc/cpuinfo");
	if(ret!=0){goto end;}
	buf=wtk_strbuf_new(64,1);
	while(1)
	{
		ret=wtk_source_seek_to2_s(&(src),"cpu MHz",buf);
		if(ret!=0){break;}
		ret=wtk_source_seek_to2_s(&(src),":",buf);
		if(ret!=0){break;}
		ret=wtk_source_skip_sp(&(src),0);
		if(ret!=0){break;}
		ret=wtk_source_read_float(&(src),&freq,1,0);
		break;
	}
	wtk_strbuf_delete(buf);
	wtk_source_clean_file(&(src));
end:
	return freq;
}
*/

/*
long long int rdtsc()
{
	__asm__("rdtsc");
	return 0;
}

float wtk_cpu_get_frequence()
{
	long long int t1,t2;
	double t3,t4;

	wtk_debug("%ld\n",sysconf(_SC_CLK_TCK));
	t3=time_get_ms();
	t1=rdtsc();
	usleep(1000000);
	t4=time_get_ms();
	t2=rdtsc();
	return (t3-t4)/(t2-t1);
}
*/

#define wtk_cpu_read_int_s(src,s,v) wtk_cpu_read_int(src,s,sizeof(s)-1,v)

int wtk_cpu_read_int(wtk_source_t *src,char *s,int s_len,int *v)
{
	int ret;

	ret=wtk_source_seek_to(src,s,s_len);
	if(ret!=0){goto end;}
	ret=wtk_source_seek_to_s(src,":");
	if(ret!=0){goto end;}
	ret=wtk_source_skip_sp(src,0);
	if(ret!=0){goto end;}
	ret=wtk_source_read_int(src,v,1,0);
end:
	return ret;
}

float wtk_cpu_get_frequence()
{
	float freq=-1;
	wtk_source_t src;
	int siblings;
	int cores;
	int ret;

	src.data=0;
	ret=wtk_source_init_file(&(src),"/proc/cpuinfo");
	if(ret!=0){goto end;}
	while(1)
	{
		ret=wtk_source_seek_to_s(&(src),"model name");
		if(ret!=0){break;}
		ret=wtk_source_seek_to_s(&(src),"@");
		if(ret!=0){break;}
		ret=wtk_source_skip_sp(&(src),0);
		if(ret!=0){break;}
		ret=wtk_source_read_float(&(src),&freq,1,0);
		break;
	}
    if (freq < 0) {
        if (src.data) {
            wtk_source_clean_file(&(src));
        }
        ret=wtk_source_init_file(&(src),"/proc/cpuinfo");
        if(ret!=0){goto end;}
        ret=wtk_source_seek_to_s(&(src),"cpu MHz");
		if(ret!=0) goto end;
		ret=wtk_source_seek_to_s(&(src),":");
		if(ret!=0) goto end;
		ret=wtk_source_skip_sp(&(src),0);
		if(ret!=0) goto end;
		ret=wtk_source_read_float(&(src),&freq,1,0);
        if (ret != 0) goto end;
        freq /= 1000;
    }
	ret=wtk_cpu_read_int_s(&(src),"siblings",&siblings);
	if(ret!=0){goto end;}
	ret=wtk_cpu_read_int_s(&(src),"cpu cores",&cores);
	if(ret!=0){goto end;}
	if(siblings!=cores)
	{
		freq*=cores*1.0/siblings;
	}
	//wtk_debug("siblings=%d,cores=%d\n",siblings,cores);
end:
	if(src.data)
	{
		wtk_source_clean_file(&(src));
	}
	return freq;
}


int wtk_cpu_ht_is_enable()
{
	wtk_source_t src;
	int siblings;
	int cores;
	int ret;
	int ht_enable;

	ht_enable=0;
	src.data=0;
	ret=wtk_source_init_file(&(src),"/proc/cpuinfo");
	if(ret!=0){goto end;}
	ret=wtk_cpu_read_int_s(&(src),"siblings",&siblings);
	if(ret!=0){goto end;}
	ret=wtk_cpu_read_int_s(&(src),"cpu cores",&cores);
	if(ret!=0){goto end;}
	ht_enable=siblings>cores;
end:
	if(src.data)
	{
		wtk_source_clean_file(&(src));
	}
	return ht_enable;
}


int wtk_get_phy_cpus()
{
	int n;
	int *p;
	int i;
	int cnt;
	int id,ret;
	wtk_source_t src;
	char *fn;

	cnt=0;
	fn="/proc/cpuinfo";
	ret=wtk_source_init_file(&(src),fn);
	if(ret!=0){goto end;}
	n=sysconf(_SC_NPROCESSORS_ONLN);
	//printf("n=%d\n",n);
	p=(int*)malloc(sizeof(int)*n);
	while(1)
	{
		ret=wtk_cpu_read_int_s(&(src),"physical id",&id);
		if(ret!=0){break;}
		for(i=0;i<cnt;++i)
		{
			if(p[i]==id)
			{
				break;
			}
		}
		if(i==cnt)
		{
			++cnt;
			p[i]=id;
		}
	}
	wtk_free(p);
end:
	if(src.data)
	{
		wtk_source_clean_file(&(src));
	}
	return cnt;
}

#endif

