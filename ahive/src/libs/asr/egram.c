#include "wtk/asr/wfst/egram/wtk_egram.h"
#include "wtk/core/cfg/wtk_mbin_cfg.h"
#include "wtk/core/cfg/wtk_opt.h"

void print_usage()
{
    printf("egram usage:\n");
    printf("\t-c configure file\n");
    printf("\t-b 1/0 resource is bin\n");
    printf("\t-i input grammar file\n");
    printf("\t-o output bin file\n");
    printf("\n");
}

static char* stdin_read_buf(int *n)
{
    wtk_strbuf_t *buf;
    int len = 0;
    int res;
    char tmp[1024];
    char *p;

    buf = wtk_strbuf_new(1024, 1);

    while ((res = fread(tmp, 1, sizeof(tmp), stdin)) > 0) {
        len += res;
        wtk_strbuf_push(buf, tmp, res);
    }
    *n = len;
    wtk_strbuf_push_c(buf, '\0');
    p = buf->data;
    buf->data = NULL;
    wtk_strbuf_delete(buf);

    return p;
}

void test_arg(char *cfg_fn,char *i_fn,char *o_fn,int bin)
{
    wtk_mbin_cfg_t *bin_cfg=NULL;
    wtk_main_cfg_t *main_cfg=NULL;
    wtk_egram_t *e;
    char *data;
    int len;
    double t;

    if (i_fn) {
        data=file_read_buf(i_fn,&(len));
    } else {
        data = stdin_read_buf(&len);
    }
    if(!data)
    {
        print_usage();
        return;
    }
    if(bin)
    {
        bin_cfg=wtk_mbin_cfg_new_type(wtk_egram_cfg,cfg_fn,
                                      "./egram.cfg.r");
        e=wtk_egram_new2(bin_cfg);
    }else
    {
        main_cfg=wtk_main_cfg_new_type(wtk_egram_cfg,cfg_fn);
        e=wtk_egram_new((wtk_egram_cfg_t*)main_cfg->cfg,NULL);
    }

    t=time_get_ms();
    wtk_egram_ebnf2fst(e,data,len);
    wtk_egram_write(e,o_fn);
    wtk_egram_write_txt(e,"out.id","out.fsm");
    t=time_get_ms()-t;

    wtk_debug("write %s, time=%f ms\n",o_fn,t);

    wtk_free(data);
    wtk_egram_delete(e);
    if(bin_cfg)
    {
        wtk_mbin_cfg_delete(bin_cfg);
    }
    if(main_cfg)
    {
        wtk_main_cfg_delete(main_cfg);
    }
}

int main(int argc,char **argv)
{
    wtk_arg_t *arg;
    char *cfg_fn;
    char *i_fn;
    char *o_fn;
    int b;

    b=0;
    arg=wtk_arg_new(argc,argv);
    if(!arg){goto end;}
    wtk_arg_get_str_s(arg,"c",&(cfg_fn));
    wtk_arg_get_str_s(arg,"i",&(i_fn));
    wtk_arg_get_str_s(arg,"o",&(o_fn));
    wtk_arg_get_int_s(arg,"b",&b);
    if(!cfg_fn || !o_fn)
    {
        print_usage();
        goto end;
    }
    test_arg(cfg_fn,i_fn,o_fn,b);
end:
    if(arg)
    {
        wtk_arg_delete(arg);
    }
    return 0;
}
