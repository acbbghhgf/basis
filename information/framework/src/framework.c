#include <mcheck.h>
#include <assert.h>
#include "wtk/os/wtk_malloc.h"
#include "qtk/frame/qtk_frame.h"
#include "qtk/util/qtk_verify.h"


static int sig_stop = 0;

static void print_version()
{
    printf("Copyright (C) Qdreamer.  All rights reserved.\nversion is %s, build at %s [%s].\n\n",STR(VER),__DATE__, STR(BUILD_HASH));
}

static void print_usage()
{
    printf("usage: speechd [options] \n\n");
    printf("Options: \n");
    printf("-h \t show this help message and exit\n");
    printf("-p \t set listen port, default is 8080\n");
    printf("-s \t stop start\n");
    printf("-v \t show version\n");
    printf("-d \t true or false (daemon)\n");
    printf("-c \t set configure file\n");
    printf("-pip \t set proxy ip like 10.12.7.128\n");
    printf("-pport \t set proxy port like 8080\n");
    printf("-debug \t set debug flag\n");
    printf("-init \t enbale init script or not\n");
    printf("\nExample:\n");
    printf("\t speechd -p 80 \n\n");
    printf("    Also you can change config file from command, like:\n-http:port 8010 will change \"http{port=8080;};\" in config file.\n\n");
}

void qtk_main_process_command(wtk_arg_t *arg)
{
    int ret;
    char *sig;

    if(wtk_arg_exist_s(arg,"h"))
    {
        print_usage();
        exit(0);
    }
    ret=wtk_arg_get_str_s(arg,"s",&sig);
    if(ret==0)
    {
        if(strcmp(sig,"stop")==0)
        {
            sig_stop = 1;
        }
    }
}

int qtk_main_run(qtk_conf_file_t *cfg)
{
    qtk_fwd_t httpd,*h=&httpd;
    int ret;

    ret=qtk_fwd_init(h,cfg);
    if(ret!=0){goto end;}
    ret=qtk_fwd_run(h);
    if(ret!=0){goto end;}
    qtk_fwd_clean(h);
end:
    return ret;
}

int main(int argc,char **argv)
{
    wtk_arg_t *arg=0;
    qtk_conf_file_t *conf=0;
    char lib[2048];
    char tmp[2048];
    int ret=-1;
    char *cfgfn=0;

    arg=wtk_arg_new(argc,argv);
    if (wtk_arg_exist_s(arg, "v")) {
        print_version();
        ret = 0;
        goto end;
    }
    wtk_arg_get_str_s(arg,"c",&cfgfn);
    qtk_main_process_command(arg);
    wtk_proc_get_dir(tmp,sizeof(tmp));
    sprintf(lib,"%s/../ext",tmp);
    conf=qtk_conf_file_new(lib,cfgfn,arg);
    if(!conf)
    {
        wtk_debug("load configure failed.\n");
        goto end;
    }
    if (sig_stop) {
        qtk_fwd_stop_proc(conf);
        exit(0);
    }
    //qtk_conf_file_update(conf,arg);
    ret=qtk_main_run(conf);
end:
    if(conf){qtk_conf_file_delete(conf);}
    if(arg){wtk_arg_delete(arg);}
    return ret;
}
