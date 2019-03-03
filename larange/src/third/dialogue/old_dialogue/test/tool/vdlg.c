#include "wtk/lex/lexc/wtk_lexc.h"
#include "wtk/core/cfg/wtk_main_cfg.h"
#include "wtk/lex/net/wtk_lex_net.h"
#include "wtk/lex/wtk_lex.h"
#include "wtk/core/cfg/wtk_opt.h"
#include "wtk/core/rbin/wtk_flist.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/json/wtk_json_parse.h"
#include "wtk/semdlg/wtk_semdlg.h"


void print_usage()
{
	printf("vdlg usgae:\n");
	printf("\t-c configure file\n");
	printf("\t-i input verify file\n");
	printf("\t-b resource is bin or not\n");
	printf("\nExample:\n");
	printf("\t ./tool/vdlg -c ./res/semdlg/semdlg.cfg -i dlg.verify \n\n");
}



void test_semdlg_delete_item(wtk_json_item_t *item,wtk_string_t *k,wtk_string_t *v)
{
	char *s,*e;
	wtk_string_t str;
	int n;

	//wtk_debug("[%.*s]\n",k->len,k->data);
	//wtk_debug("item=========%p\n", item);
	s=k->data;
	e=s+k->len;
	str.data=s;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		if(n==1 && *s=='.')
		{
			str.len=s-str.data;
			//wtk_debug("[%.*s]\n",str.len,str.data);
			if(item)
			{
				item=wtk_json_obj_get(item,str.data,str.len);
				//wtk_debug("get item = %p, str = %.*s\n", item, str.len, str.data);
			}
			str.data=s+1;
		}
		s+=n;
	}
	str.len=s-str.data;
	//wtk_debug("[%.*s]\n",str.len,str.data);
	wtk_json_obj_remove(item,str.data,str.len);
	//wtk_json_item_print3(item);
	//exit(0);
}


void test_semdlg_xy(wtk_semdlg_t *dlg,wtk_string_t *k,wtk_string_t *v)
{
	//wtk_debug("[%.*s]\n",k->len,k->data);
	if(wtk_string_cmp_s(k,"fld")==0)
	{
		wtk_semdlg_set_fld(dlg,v->data,v->len);
	}else if(wtk_string_cmp_s(k,"bg")==0)
	{
		wtk_semdlg_set_bg(dlg,1);
	}else if(wtk_string_cmp_s(k,"fg")==0)
	{
		wtk_semdlg_set_bg(dlg,0);
	}else if(wtk_string_cmp_s(k,"reset")==0)
	{
		//wtk_debug("===============> reset\n");
		wtk_semdlg_reset_history(dlg);
	}else if(wtk_string_cmp_s(k,"lua")==0)
	{
		//wtk_debug("[%.*s]=[%.*s]\n",k->len,k->data,v->len,v->data);
		wtk_semdlg_update_lua(dlg,v->data,v->len);
		//exit(0);
	}else if(wtk_string_cmp_s(k,"env")==0)
	{
		wtk_debug("[%.*s]=[%.*s]\n",k->len,k->data,v->len,v->data);
		wtk_semdlg_set_env(dlg,v->data,v->len);
	}else if(wtk_string_cmp_s(k,"del")==0)
	{
		wtk_debug("[%.*s]=[%.*s]\n",k->len,k->data,v->len,v->data);
		if(v)
		{
			wtk_strbuf_t *buf;

			buf=wtk_strbuf_new(256,1);
			wtk_strbuf_reset(buf);
			wtk_strbuf_push(buf,v->data,v->len);
			wtk_strbuf_push_c(buf,0);
			remove(buf->data);
			wtk_strbuf_delete(buf);
		}
		//exit(0);
	}else if(wtk_string_cmp_s(k,"random")==0)
	{
		wtk_semdlg_set_random(dlg,0);
//		if(dlg->actkg)
//		{
//			dlg->actkg->owlkv->cfg->use_random=wtk_str_atoi(v->data,v->len);
//		}
	}else if(wtk_string_cmp_s(k,"sleep")==0)
	{
		int i;

		i=wtk_str_atoi(v->data,v->len);
		wtk_msleep(i*1000);
	}
	else
	{
		if(v)
		{
			wtk_debug("[%.*s]=[%.*s]\n",k->len,k->data,v->len,v->data);
		}
		//exit(0);
	}
}


void wtk_semdlg_process_file(wtk_semdlg_t *dlg,char *s)
{
	static int run=1;
	wtk_string_t input;
	wtk_string_t attr;
	wtk_string_t v;
	int i;
	wtk_json_parser_t* px;
	wtk_json_item_t *mainx;
	int ret;
	int check=1;
	wtk_strbuf_t *buf;

	if(!run)
	{
		return;
	}
	if(s[0]=='/' && s[1]=='/')
	{
		s=s+2;
		wtk_semdlg_update_lua(dlg,s,strlen(s));
		return;
	}
//	wtk_json_parser_parse(px,ps,strlen(ps));
//	wtk_json_item_print3(px->json->main);
//	exit(0);

	//if(]=='#')
	{
		if(wtk_str_equal_s(s,strlen(s),"#xexit"))
		{
			exit(0);
			run=0;
			return;
		}
		if(wtk_str_equal_s(s,strlen(s),"#exit"))
		{
			//exit(0);
			run=0;
			return;
		}else if(wtk_str_equal_s(s,strlen(s),"#skipSession"))
		{
			wtk_semdlg_skip_session(dlg);
			return;
		}else if(s[0]=='#')
		{
			wtk_strbuf_t *buf;

			buf=wtk_strbuf_new(256,1);
			wtk_strbuf_push_s(buf,"[");
			wtk_strbuf_push(buf,s+1,strlen(s)-1);
			wtk_strbuf_push_s(buf,"]");

			wtk_debug("[%.*s]\n",buf->pos,buf->data);
			wtk_str_attr_parse(buf->data,buf->pos,dlg,(wtk_str_attr_f)test_semdlg_xy);
			wtk_strbuf_delete(buf);
			//exit(0);
			return;
		}
	}
	px=wtk_json_parser_new();
	printf("INPUT=> %s\n",s);
	i=wtk_str_str(s,strlen(s),"=>",2);
	//wtk_debug("i=%d\n",i);
	//wtk_debug("[%.*s]\n",i,s);
	if(i>0)
	{
		wtk_string_set(&(input),s,i);
		if(input.data[0]=='#')
		{
			++input.data;
			--input.len;
			check=0;
		}
		buf=px->buf;
		wtk_strbuf_reset(buf);
		wtk_strbuf_push(buf,input.data,input.len);
		wtk_strbuf_strip(buf);
		wtk_string_set(&(input),buf->data,buf->pos);
		wtk_string_set(&(attr),s+i+2,strlen(s)-i-2);
	}else
	{
		wtk_string_set(&(input),s,strlen(s));
		wtk_string_set(&(attr),0,0);
		check=0;
	}
	//wtk_debug("[%.*s]\n",input.len,input.data);
//	wtk_debug("[%.*s]\n",attr.len,attr.data);
//	wtk_debug("v=%d\n",i);

	dlg->conf=3;
	if(input.data[0]=='{' && input.data[input.len-1]=='}')
	{
		wtk_semdlg_feed_json(dlg,input.data,input.len);
	}else
	{
		wtk_semdlg_process(dlg,input.data,input.len);
	}
	//sys
	/*
	wtk_strbuf_t *the_buf = wtk_strbuf_new(1024, 1);
	wtk_strbuf_t *s_buf = wtk_strbuf_new(1024, 1);
	qtk_semdlg_get_inter_state(dlg, s_buf);
	qtk_lua_local_get(dlg, the_buf);
	//wtk_debug("--------------> %.*s\n", the_buf->pos, the_buf->data);
	qtk_semdlg_set_inter_state(dlg, s_buf->data, s_buf->pos);
	qtk_lua_local_set(dlg, the_buf->data, the_buf->pos);
	wtk_strbuf_delete(the_buf);
	wtk_strbuf_delete(s_buf);
	*/
	v=wtk_semdlg_get_result(dlg);
	mainx=wtk_json_obj_get_s(dlg->output,"output");
	if(mainx)
	{
		printf("OUPUT=> %.*s  %.*s\n",mainx->v.str->len,mainx->v.str->data,v.len,v.data);
	}else
	{
		printf("OUPUT=> NIL  %.*s\n",v.len,v.data);
	}

	if(check && attr.len>0)
	{
		wtk_json_obj_remove_s(dlg->output,"output");
		wtk_json_obj_remove_s(dlg->output,"output2");
		//wtk_debug("[%.*s]\n",attr.len,attr.data);
		i=wtk_str_str(attr.data,attr.len,"#",1);
		if(i>=0)
		{
			//wtk_debug("[%.*s]\n",attr.len-i-1,attr.data+i+1);
			wtk_string_set(&(input),attr.data+i+1,attr.len-i-1);
			attr.len=i-1;
		}else
		{
			wtk_string_set(&(input),0,0);
		}
		//wtk_debug("[%.*s]\n",attr.len,attr.data);
		//wtk_debug("[%.*s]\n",input.len,input.data);
		//exit(0);
		ret=wtk_json_parser_parse(px,attr.data,attr.len);
		if(ret!=0)
		{
			wtk_debug("=========== err ===========\n");
			wtk_debug("invalid json.\n");
			exit(0);
		}
		mainx=px->json->main;
		if(mainx && dlg->output)
		{
			wtk_json_obj_remove_s(mainx,"output");
			wtk_json_obj_remove_s(mainx,"output2");
			if(input.len>0)
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push_s(buf,"[");
				wtk_strbuf_push(buf,input.data,input.len);
				wtk_strbuf_push_s(buf,"]");
				wtk_string_set(&(input),buf->data,buf->pos);
				//wtk_debug("[%.*s]\n",input.len,input.data);
				wtk_str_attr_parse(input.data,input.len,mainx,(wtk_str_attr_f)test_semdlg_delete_item);
				wtk_str_attr_parse(input.data,input.len,dlg->output,(wtk_str_attr_f)test_semdlg_delete_item);
				//exit(0);
			}
			ret=wtk_json_item_cmp(mainx,dlg->output);
		}else
		{
			wtk_debug("output is invalid\n");
			ret=-1;
		}
		if(ret!=0)
		{
			wtk_debug("=========== err ===========\n");
			wtk_debug("invalid.\n");
			wtk_json_item_print3(dlg->output);
			wtk_json_item_print3(mainx);
			exit(0);
		}
	}
	wtk_semdlg_reset(dlg);
	wtk_json_parser_delete(px);
}

wtk_string_t vdlg_test_faq(void *dlg,char *data,int len)
{
	wtk_string_t v;

	//wtk_debug("[%.*s]\n",len,data);
	if(wtk_str_equal_s(data,len,"今天天气很好"))
	{
		wtk_string_set_s(&(v),"这是一条测试数据");
	}else
	{
		wtk_string_set(&(v),0,0);
	}
	return v;
}


int main(int argc,char **argv)
{
	wtk_semdlg_cfg_t *cfg=NULL;
	wtk_semdlg_t *dlg=NULL;
	wtk_arg_t *arg;
	char *cfg_fn=NULL;
	char *input_fn=NULL;
	double b=0;

	arg=wtk_arg_new(argc,argv);
	if(!arg)
	{
		print_usage();
		goto end;
	}
	wtk_arg_get_str_s(arg,"c",&cfg_fn);
	wtk_arg_get_number_s(arg,"b",&b);
	wtk_arg_get_str_s(arg,"i",&input_fn);
	if(!cfg_fn || !input_fn)
	{
		print_usage();
		goto end;
	}
	wtk_debug("b=%f\n",b);
	if(b>0)
	{
		cfg=wtk_semdlg_cfg_new_bin(cfg_fn);
	}else
	{
		cfg=wtk_semdlg_cfg_new(cfg_fn);
	}
	//wtk_debug("load configure =%p\n",cfg);
	if(!cfg)
	{
		goto end;
	}
	dlg=wtk_semdlg_new(cfg);
	wtk_semdlg_set_lua_dat(dlg,"direction","{\"theta\":50}");
	//getchar();
	//wtk_semdlg_set_faq_get(dlg,NULL,(wtk_semdlg_get_faq_f)vdlg_test_faq);
	//void wtk_semdlg_set_faq_get(wtk_semdlg_t *dlg,void *ths,wtk_semdlg_get_faq_f faq_get)
	if(input_fn)
	{
		wtk_flist_process(input_fn,dlg,(wtk_flist_notify_f)wtk_semdlg_process_file);
		printf("\n");
		wtk_debug("============= success ============\n");
	}
	//wtk_debug("delete dlg\n");
	wtk_semdlg_delete(dlg);
end:
	if(cfg)
	{
		wtk_semdlg_cfg_delete(cfg);
	}
	if(arg)
	{
		wtk_arg_delete(arg);
	}
	return 0;
}
