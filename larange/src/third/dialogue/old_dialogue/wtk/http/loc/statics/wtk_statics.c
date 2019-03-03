#ifdef USE_STATICS
#include "wtk/http/nk/wtk_nk.h"
#include "wtk_statics.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/os/wtk_proc.h"
#include "wtk/http/wtk_http.h"
#ifndef WIN32
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#endif

static wtk_speech_t *loc_speech;
static wtk_vm_t *loc_vm;

void wtk_loc_statics_set_vars(wtk_speech_t *speech,wtk_vm_t *vm)
{
	loc_speech=speech;
	loc_vm=vm;
}

void wtk_loc_attach_statics(wtk_stack_t *stk,wtk_nk_t *nk)
{
	int n;
	char tmp[1024];

#ifdef WIN32
    wtk_stack_push_f(stk, "{\"cpu\":{\"ncpu\":%d}",wtk_get_cpus());
#else
	wtk_stack_push_f(stk,"{\"time\":%.3f,\"cpu\":",nk->log->t->wtk_cached_time);
	wtk_cpu_to_string2(nk->cpu,stk);
#endif
	if(loc_vm)
	{
        n=sprintf(tmp,",\"vm\":{\"scripts\":%d,\"speechUp\":%d,\"speechDown\":%d}",loc_vm->script_hoard->use_length,
        		loc_vm->speech_queue->length,loc_vm->speech_to_vm_queue->length);
        wtk_stack_push(stk,tmp,n);
	}
	wtk_stack_push_s(stk,"}");
}

void wtk_loc_attach_statics2(wtk_stack_t *stk,wtk_nk_t *nk)
{
	int n;
	char tmp[1024];

#ifdef WIN32
    wtk_stack_push_f(stk, "{\"cpu\":{\"ncpu\":%d}",wtk_get_cpus());
#else
	wtk_stack_push_f(stk,"time=%.3f;cpu=",nk->log->t->wtk_cached_time);
	wtk_cpu_to_string3(nk->cpu,stk);
	wtk_stack_push_s(stk,";");
#endif
	if(loc_vm)
	{
        n=sprintf(tmp,"vm={scripts=%d;speechUp=%d;speechDown=%d;};",loc_vm->script_hoard->use_length,
        		loc_vm->speech_queue->length,loc_vm->speech_to_vm_queue->length);
        wtk_stack_push(stk,tmp,n);
	}
}

int wtk_loc_statics_process(void *data,wtk_request_t *req)
{
	wtk_nk_t *net;
	wtk_strbuf_t *buf;
	int ret;
	char tmp[1024];
	int n;
#ifndef WIN32
	struct sysinfo info;
#endif
	double v;
    wtk_http_cfg_t *cfg;
	wtk_http_t *http;

	if(!loc_speech)
	{
		wtk_response_set_body_s(req->response,"statics not suported.");
		return 0;
	}
	net=req->c->net;
    http=(wtk_http_t*)(((wtk_http_parser_t*)(req->c->parser))->layer);
    cfg=http->cfg;
	buf=net->tmp_buf;
	wtk_strbuf_reset(buf);
	wtk_strbuf_push_s(buf,"{\"name\":\"http\",\"con\":");
	n=sprintf(tmp,"%d",net->con_hoard->use_length-1);
	wtk_strbuf_push(buf,tmp,n);
#ifndef WIN32
    ret=sysinfo(&info);
    if(ret==0)
    {
        v=(info.mem_unit*info.freeram)/(1024*1024);
        n=sprintf(tmp,",\"memFree\":%.0f",v);
        wtk_strbuf_push(buf,tmp,n);
        v=(info.mem_unit*info.totalram)/(1024*1024);
        n=sprintf(tmp,",\"memTot\":%.0f,\"memProc\":%.0f",v,wtk_proc_mem());
        wtk_strbuf_push(buf,tmp,n);
    }
#endif
    wtk_strbuf_push_s(buf,",\"version\":\"");
    wtk_strbuf_push(buf,cfg->ver.data,cfg->ver.len);
    wtk_strbuf_push_s(buf,"\"");
    //ret=wtk_get_cpus();
    //n=sprintf(tmp,",\"ncpu\":%d",ret);
    //wtk_strbuf_push(buf,tmp,n);
    if(loc_vm)
    {
        http=(wtk_http_t*)((wtk_http_parser_t*)(req->c->parser))->layer;
        n=sprintf(tmp,",\"http\":{\"requests\":%d,\"vmUp\":%d,\"vmDown\":%d}",http->request_hoard->use_length,
        		loc_vm->request_queue->length,http->net->pipe->pipe_queue.length);
        wtk_strbuf_push(buf,tmp,n);
        n=sprintf(tmp,",\"vm\":{\"scripts\":%d,\"speechUp\":%d,\"speechDown\":%d}",loc_vm->script_hoard->use_length,
        		loc_vm->speech_queue->length,loc_vm->speech_to_vm_queue->length);
        wtk_strbuf_push(buf,tmp,n);
    	//wtk_strbuf_push_s(buf,"\"vm\":");
    }
    //wtk_strbuf_push_f(buf,",\"mem\":{\"http\":%d,\"vm\":%d}",wtk_http_bytes(http),wtk_vm_bytes(loc_vm));
    if(loc_speech)
    {
    	wtk_strbuf_push_s(buf,",\"speech\":");
    	wtk_speech_get_info(loc_speech,buf,1);
    }
#ifndef WIN32
    if(net->cpu)
    {
    	wtk_cpu_t *cpu;
    	int i;

    	cpu=net->cpu;
    	//wtk_cpu_update(cpu);
    	wtk_strbuf_push_s(buf,",\"cpu\":{");
    	for(i=wtk_cpu_user;i<=wtk_cpu_guest;++i)
    	{
    		if(i>wtk_cpu_user)
    		{
    			wtk_strbuf_push_s(buf,",");
    		}
    		wtk_strbuf_push_f(buf,"\"%*.*s\":%.1f",cpu_names[i].len,cpu_names[i].len,cpu_names[i].data,
    				cpu->tot==0?0:cpu->diff[i]*100/cpu->tot);
    	}
    	wtk_strbuf_push_f(buf,",\"tot\":%.1f,\"ncpu\":%d",cpu->tot,cpu->ncpu);
    	wtk_strbuf_push_s(buf,"}");
    }
#endif
    wtk_strbuf_push_s(buf,"}");
    wtk_response_set_body(req->response,buf->data,buf->pos);
	return 0;
}

int wtk_loc_debug_process(void *data,wtk_request_t *req)
{
	wtk_strbuf_t *buf;
	wtk_http_t *http;
#ifdef WIN32
#else
	wtk_statm_t m;
#endif
	//wtk_print_statm();
	if(!loc_speech)
	{
		wtk_response_set_body_s(req->response,"debug not suported by proxy.");
		return 0;
	}
    http=(wtk_http_t*)(((wtk_http_parser_t*)(req->c->parser))->layer);
	buf=http->net->tmp_buf;
	wtk_strbuf_reset(buf);
	wtk_strbuf_push_f(buf,"{\"cpu\":%d,\"time-cur\":%.0f,\"vm\":",loc_speech->cfg->cpus,time_get_ms());
	wtk_vm_debug(loc_vm,buf);
	wtk_strbuf_push_s(buf,",\"speech\":");
	wtk_speech_get_info(loc_speech,buf,1);
#ifdef WIN32
#else
	wtk_statm_init(&m);
	wtk_strbuf_push_f(buf,",\"mem\":{\"http\":%.3f,\"vm\":%.3f,\"speech\":%.3f,\"size\":%.3f,\"resident\":%.3f}",
			wtk_http_bytes(http)*1.0/(1024*1024),
			wtk_vm_bytes(loc_vm)*1.0/(1024*1024),
			wtk_speech_bytes(loc_speech)*1.0/(1024*1024),
			m.size*4.0/1024,m.resident*4.0/1024);
#endif
	wtk_strbuf_push_s(buf,"}");
	//wtk_debug("%.*s\n",buf->pos,buf->data);
	wtk_response_set_body(req->response,buf->data,buf->pos);
	return 0;
}


int wtk_loc_speech_process(void *data,wtk_request_t *req)
{
	wtk_strbuf_t *buf;

	if(!loc_speech)
	{
		wtk_response_set_body_s(req->response,"speech not suported by proxy.");
		return 0;
	}
	buf=req->c->net->tmp_buf;
	wtk_strbuf_reset(buf);
	wtk_strbuf_push_s(buf,"{\"speech\":");
	wtk_speech_get_info(loc_speech,buf,0);
	wtk_strbuf_push_s(buf,"}");
	wtk_response_set_body(req->response,buf->data,buf->pos);
	return 0;
}

int wtk_loc_cpu_process(void *data,wtk_request_t *req)
{
	wtk_nk_t *net;
	wtk_strbuf_t *buf;
	int ret;
	char tmp[1024];
	int n;
#ifndef WIN32
	struct sysinfo info;
#endif
	double v;

	net=req->c->net;
	buf=net->tmp_buf;
	wtk_strbuf_reset(buf);
	wtk_strbuf_push_s(buf,"{\"con\":");
	n=sprintf(tmp,"%d",net->con_hoard->use_length-1);
	wtk_strbuf_push(buf,tmp,n);
#ifndef WIN32
    ret=sysinfo(&info);
    if(ret==0)
    {
        v=(info.mem_unit*info.freeram)/(1024*1024);
        n=sprintf(tmp,",\"memFree\":%.0f",v);
        wtk_strbuf_push(buf,tmp,n);
        v=(info.mem_unit*info.totalram)/(1024*1024);
        n=sprintf(tmp,",\"memTot\":%.0f",v);
        wtk_strbuf_push(buf,tmp,n);
    }
    /*
	wtk_strbuf_push_f(buf,",\"mem\":{\"http\":%.3f,\"vm\":%.3f,\"speech\":%.3f,\"proc\":%.3f}",
			wtk_http_bytes((wtk_http_t*)(((wtk_http_parser_t*)(req->c->parser))->layer))*1.0/(1024*1024),
			wtk_vm_bytes(loc_vm)*1.0/(1024*1024),
			wtk_speech_bytes(loc_speech)*1.0/(1024*1024),
			wtk_proc_mem());
	*/
#endif
    ret=wtk_get_cpus();
    n=sprintf(tmp,",\"ncpu\":%d",ret);
    wtk_strbuf_push(buf,tmp,n);
#ifndef WIN32
    if(net->cpu)
    {
    	wtk_cpu_t *cpu;
    	int i;

    	cpu=net->cpu;
    	//wtk_cpu_update(cpu);
    	wtk_strbuf_push_s(buf,",\"cpu\":{");
    	for(i=wtk_cpu_user;i<=wtk_cpu_guest;++i)
    	{
    		if(i>wtk_cpu_user)
    		{
    			wtk_strbuf_push_s(buf,",");
    		}
    		wtk_strbuf_push_f(buf,"\"%*.*s\":%.1f",cpu_names[i].len,cpu_names[i].len,cpu_names[i].data,cpu->rate[i]);
    	}
    	wtk_strbuf_push_f(buf,",\"tot\":%.1f",cpu->tot);
    	wtk_strbuf_push_s(buf,"}");
    }
#endif
    wtk_strbuf_push_s(buf,"}");
    wtk_response_set_body(req->response,buf->data,buf->pos);
	return 0;
}
#endif
