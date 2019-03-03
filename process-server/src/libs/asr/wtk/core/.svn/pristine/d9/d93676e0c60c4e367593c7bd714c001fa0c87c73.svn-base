#include "wtk_opt.h"

void wtk_opt_print_usage()
{
	printf("\t-c configure file\n");
	printf("\t-s input string\n");
	printf("\t-i input file\n");
	printf("\t-o output file\n");
}

void wtk_opt_update_arg(wtk_opt_t *opt)
{
	wtk_arg_t *arg=opt->arg;

	wtk_arg_get_str_s(arg,"c",&(opt->cfg_fn));
	wtk_arg_get_str_s(arg,"s",&(opt->input_s));
	wtk_arg_get_str_s(arg,"i",&(opt->input_fn));
	wtk_arg_get_str_s(arg,"o",&(opt->output_fn));
}

wtk_opt_t* wtk_opt_new(int argc,char **argv,int cfg_bytes,
		wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,
		wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update
		)
{
	wtk_opt_t *opt;
	int ret=-1;

	opt=(wtk_opt_t*)wtk_malloc(sizeof(wtk_opt_t));
	opt->cfg_fn=NULL;
	opt->input_fn=NULL;
	opt->output_fn=NULL;
	opt->input_s=NULL;
	opt->output_fn=NULL;
	opt->arg=NULL;
	opt->main_cfg=NULL;
	opt->arg=wtk_arg_new(argc,argv);
	opt->output=NULL;
	opt->log=NULL;
	wtk_opt_update_arg(opt);
	if(opt->cfg_fn)
	{
		opt->main_cfg=wtk_main_cfg_new4(cfg_bytes,init,clean,update_lc,update,opt->cfg_fn,opt->arg);
		if(!opt->main_cfg)
		{
			printf("load configure[%s] failed.\n",opt->cfg_fn);
			goto end;
		}
	}
	if(opt->output_fn && 0)
	{
		opt->output=fopen(opt->output_fn,"w");
		if(opt->output)
		{
			opt->log=opt->output;
		}else
		{
			opt->log=stdout;
		}
	}else
	{
		opt->log=stdout;
	}
	ret=0;
end:
	if(ret!=0)
	{
		wtk_opt_delete(opt);
		opt=NULL;
	}
	return opt;
}

void wtk_opt_delete(wtk_opt_t *opt)
{
	if(opt->output)
	{
		fclose(opt->output);
	}
	if(opt->main_cfg)
	{
		wtk_main_cfg_delete(opt->main_cfg);
	}
	if(opt->arg)
	{
		wtk_arg_delete(opt->arg);
	}
	wtk_free(opt);
}
