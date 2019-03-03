#include "wtk/lex/lexc/wtk_lexc.h"
#include "wtk/core/cfg/wtk_main_cfg.h"
#include "wtk/lex/net/wtk_lex_net.h"
#include "wtk/lex/wtk_lex.h"
#include "wtk/core/cfg/wtk_opt.h"
#include "wtk/core/rbin/wtk_flist.h"


void print_usage()
{
	printf("lex usgae:\n");
	printf("\t-c configure file\n");
	printf("\t-l lex file\n");
	printf("\t-s input string\n");
	printf("\t-i input file\n");
	printf("\nExample:\n");
	printf("\t ./tool/lex -c ./res/lex.cfg -l test.lex -s \"我在上海\"\n\n");
}

void wtk_lex_process_file(wtk_lex_t *l,char *s)
{
	wtk_string_t v;

	if(s[0]=='#')
	{
		printf("%s\n",s);
		return;
	}
	printf("INPUT=> %s\n",s);
	v=wtk_lex_process(l,s,strlen(s));
	printf("OUPUT=> %.*s\n",v.len,v.data);
	fflush(stdout);
	wtk_lex_reset(l);
}

int main(int argc,char **argv)
{
	wtk_opt_t *opt;
	wtk_lex_t *l;
	char *lex_f;
	int ret;

	opt=wtk_opt_new_type(argc,argv,wtk_lex_cfg);
	if(!opt || !opt->main_cfg)
	{
		print_usage();
		goto end;
	}
	l=wtk_lex_new((wtk_lex_cfg_t*)(opt->main_cfg->cfg));
	wtk_arg_get_str_s(opt->arg,"l",&lex_f);
	if(!lex_f ||(!opt->input_s && !opt->input_fn))
	{
		print_usage();
		goto end;
	}
	ret=wtk_lex_compile(l,lex_f);
	if(ret!=0)
	{
		wtk_debug("compile file failed.\n");
		goto end;
	}
	if(opt->input_s)
	{
		wtk_lex_process_file(l,opt->input_s);
	}
	if(opt->input_fn)
	{
		wtk_flist_process4(opt->input_fn,l,(wtk_flist_notify_f)wtk_lex_process_file);
	}
	wtk_lex_delete(l);
end:
	if(opt)
	{
		wtk_opt_delete(opt);
	}
	return 0;
}
