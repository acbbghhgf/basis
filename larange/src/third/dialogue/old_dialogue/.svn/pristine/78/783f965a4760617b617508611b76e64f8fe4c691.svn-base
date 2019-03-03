#include "wtk_lua2.h"
#include <ctype.h>
#include <stdarg.h>
#ifndef WIN32
#include <regex.h>
#endif
#include <locale.h>
int wtk_lua2_load_file(wtk_lua2_t *lua2, lua_State *state, char *fn);

#ifndef WIN32
static int lua_rematch(lua_State *l)
{
	const char *var;
	const char *pattern;
	int nmatch;
	regmatch_t *m=0;
	regex_t reg,*preg;
	int ret;
	int i;
	int n=0;

	preg=0;
	i=1;
	if(!lua_isstring(l,i)){goto end;}
	var=lua_tostring(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	pattern=lua_tostring(l,i);
	nmatch=wtk_str_char_count(pattern,'(');
	nmatch+=1;
	m=(regmatch_t*)wtk_calloc(nmatch,sizeof(regmatch_t));
	ret=regcomp(&(reg),pattern,REG_EXTENDED);
	if(ret!=0){goto end;}
	preg=&(reg);
	ret=regexec(&(reg),var,nmatch,m,0);
	if(ret!=0){goto end;}
	//wtk_debug("%s\n",var);
	for(i=1;i<nmatch && m[i].rm_so>=0;++i)
	{
		//"我要坐公交去苏州"
		//wtk_debug("s=%d,e=%d\n",m[i].rm_so,m[i].rm_eo);
		//wtk_debug("v[%d]=[%.*s]\n",i,m[i].rm_eo-m[i].rm_so,&(var[m[i].rm_so]));
		lua_pushlstring(l,&(var[m[i].rm_so]),m[i].rm_eo-m[i].rm_so);
		++n;
	}
	//wtk_debug("ret=%d\n",ret);
end:
	if(m)
	{
		wtk_free(m);
	}
	if(preg)
	{
		regfree(&(reg));
	}
	//wtk_debug("n=%d\n",n);
	return n;
}

void wtk_lua_init_libs(lua_State *state)
{
	lua_pushcfunction(state,lua_rematch);
	lua_setglobal(state,"rematch");
}
#endif

void wtk_lua2_link_function(wtk_lua2_t *lua,lua_CFunction func,char *name)
{
	lua_State *state=lua->state;

	lua_pushcfunction(state,func);
	lua_setglobal(state,name);
}

#include "wtk/core/wtk_os.h"
#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
#ifndef _UNISTD_H
#define _UNISTD_H
#include <io.h>
#include <process.h>
#endif /* _UNISTD_H */
#else
#include <unistd.h>
#endif
#include <dirent.h>


int wtk_lua2_include_dir_file_name(void **ths,char *fn)
{
#define SUF ".lua"
	int len;

	len=strlen(fn);
	if(len>sizeof(SUF)&&strcmp(fn+len-sizeof(SUF)+1,SUF)==0)
	{
		return wtk_lua2_load_file((wtk_lua2_t*)ths[0],(lua_State*)ths[1],fn);
	}
	return 0;
}

int wtk_lua2_include_dir_file(wtk_lua2_t *lua2,lua_State *state,char *dir)
{
	void *ths[2];

	ths[0] = lua2;
	ths[1] = state;
#ifdef WIN32
	return wtk_dir_walk(dir, ths, (wtk_dir_walk_handler_t)wtk_lua2_include_dir_file_name);
#else
	return wtk_dir_walk2(dir, ths, (wtk_dir_walk2_f)wtk_lua2_include_dir_file_name);
#endif
}



int wtk_lua2_include_dir_file2(wtk_lua2_t *lua2,lua_State *state,char *dir)
{
	DIR *d;
	struct dirent *ent;
	int ret;
	wtk_strbuf_t *buf;

	//wtk_debug("import dir:%s\n",dir);
	//dir=wtk_heap_dup_str(re->label->hash->heap,dir);
	buf=wtk_strbuf_new(1024,1);
	d=opendir(dir);
	if(!d){ret=-1;goto end;}
	ret=0;
	while(1)
	{
		ent=readdir(d);
		if(!ent){break;}
		//wtk_debug("[%s]\n",ent->d_name);
		if(ent->d_type==DT_REG)
		{
			wtk_strbuf_reset(buf);
			//wtk_strbuf_push_string(buf,dir);
			wtk_strbuf_push(buf,dir,strlen(dir));
			wtk_strbuf_push_c(buf,'/');
			wtk_strbuf_push_string(buf,ent->d_name);
			wtk_strbuf_push_c(buf,0);
			//wtk_debug("%s\n",buf->data);
			//wtk_debug("cmp: %s:%d\n",buf->data+buf->pos-sizeof(".lex"),strcmp(buf->data+buf->pos-sizeof(".lex"),".lex"));
			if(buf->pos>sizeof(".lua") && strcmp(buf->data+buf->pos-sizeof(".lua"),".lua")==0)
			{
				//wtk_strbuf_reset(re->as);
				//wtk_strbuf_push(re->as,ent->d_name,strlen(ent->d_name)-sizeof(".lex")+1);
				//wtk_debug("[%.*s]\n",re->as->pos,re->as->data);
				//exit(0);
				ret = wtk_lua2_load_file(lua2, state, buf->data);
			}
			else
			{
				ret=0;
			}
		}
		else if (ent->d_type == DT_DIR)
		{
			if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
			{
				ret = 0;
			}
			else
			{
				wtk_strbuf_reset(buf);
				//wtk_strbuf_push_string(buf,dir);
				wtk_strbuf_push(buf,dir,strlen(dir));
				wtk_strbuf_push_c(buf,'/');
				wtk_strbuf_push_string(buf,ent->d_name);
				wtk_strbuf_push_c(buf,0);
				//wtk_debug("%s[%s:%s]\n",buf->data,ent->d_name,dir);
				ret=wtk_lua2_load_file(lua2,state,buf->data);
			}
		}
		else
		{
			ret=0;
		}
		//wtk_debug("ret=%d\n",ret);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	//exit(0);
	if(d)
	{
		closedir(d);
	}
	//wtk_debug("ret=%d\n",ret);
	wtk_strbuf_delete(buf);
	return ret;
}

int wtk_lua2_include_dir_rbin(wtk_lua2_t *lua2,lua_State *state,char *dir)
{
	int ret;
	wtk_strbuf_t *buf;
	wtk_rbin2_t *rbin=lua2->rbin;
	wtk_queue_node_t *qn;
	wtk_rbin2_item_t *item;
	wtk_string_t v;
	wtk_string_t *vx;

	buf=wtk_strbuf_new(256,1);
	vx=wtk_string_dup_data(dir,strlen(dir));
	//wtk_debug("import dir:%s\n",dir);
	for(qn=rbin->list.pop;qn;qn=qn->next)
	{
		item=data_offset(qn,wtk_rbin2_item_t,q_n);
		//wtk_debug("[%.*s],pos=%d,len=%d\n",item->fn->len,item->fn->data,item->pos,item->len);
		v=wtk_dir_name2(item->fn->data,item->fn->len,'/');
		//wtk_debug("[%.*s]=[%s]\n",v.len,v.data,dir);
		if(wtk_str_equal(v.data,v.len,vx->data,vx->len) && wtk_str_end_with_s(item->fn->data,item->fn->len,".lua"))
		{
			//wtk_debug("[%.*s],pos=%d,len=%d\n",item->fn->len,item->fn->data,item->pos,item->len);
			wtk_strbuf_reset(buf);
			wtk_strbuf_push(buf,item->fn->data,item->fn->len);
			wtk_strbuf_push_c(buf,0);
			ret=wtk_lua2_load_file(lua2,state,buf->data);
			//wtk_debug("ret=%d\n",ret);
			if(ret!=0){goto end;}
		}
	}
	ret=0;
end:
	wtk_string_delete(vx);
	//exit(0);
	wtk_strbuf_delete(buf);
	return ret;
}


int wtk_lua2_include_dir(wtk_lua2_t *lua2,lua_State *state,char *dir)
{
	if(lua2->rbin)
	{
		return wtk_lua2_include_dir_rbin(lua2, state, dir);
	}
	else
	{
		return wtk_lua2_include_dir_file(lua2,state,dir);
	}
}



int wtk_lua2_load_file(wtk_lua2_t *lua2,lua_State *state,char *fn)
{
	int ret;

	//wtk_debug("load [%s]\n",fn);
	if(lua2->rbin)
	{
		if(!wtk_str_end_with_s(fn,strlen(fn),".lua"))
		{
			ret=wtk_lua2_include_dir(lua2,state,fn);
			return ret;
		}
	}
	else
	{
		if(wtk_is_dir(fn))
		{
			return wtk_lua2_include_dir(lua2,state,fn);
		}
	}
	if(lua2->rbin)
	{
		wtk_rbin2_item_t *item;

		item=wtk_rbin2_get3(lua2->rbin,fn,strlen(fn),0);
		if(item->data && item->data->len>0)
		{
			ret = luaL_loadbuffer(state, item->data->data, item->data->len, "str");
		}
		else
		{
			wtk_rbin2_item_clean(item);
			return 0;
		}
		wtk_rbin2_item_clean(item);
	}
	else
	{
		ret=luaL_loadfile(state,fn);
	}
	//wtk_debug("load file: %s\n",fn);
	//wtk_debug("ret=%d;%s\n",ret,strs[i]->data);
	if(ret!=0)
	{
		wtk_debug("%s\n",lua_tostring(state,-1));
		goto end;
	}
	ret=lua_pcall(state,0,0,0);
	//wtk_debug("ret=%d\n",ret);
	if(ret!=0)
	{
		wtk_debug("%s %s\n",lua_tostring(state,-1),fn);
		goto end;
	}
end:
	//exit(0);
	//wtk_debug("ret=%d\n",ret);
	return ret;
}

int wtk_lua2_load_file2(wtk_lua2_t *lua2,char *fn)
{
	return wtk_lua2_load_file(lua2,lua2->state,fn);
}

static int lua_State_load_run_string(lua_State *s, char *data, int len, char *name)
{
	int ret;

	ret = luaL_loadbuffer(s, data, len, name);
	if (ret != 0)
	{
		wtk_debug("%s\n", lua_tostring(s,-1));
		goto end;
	}
	ret = lua_pcall(s, 0, 0, 0);
	if (ret != 0)
	{
		wtk_debug("%s\n", lua_tostring(s,-1));
		goto end;
	}
	end: return ret;
}

int wtk_lua2_load_str(wtk_lua2_t *lua2,char *data,int bytes)
{
	return lua_State_load_run_string(lua2->state, data,bytes, "string");
}

char* wtk_lua_rbin_search_file(wtk_rbin2_t *rbin,char *fn,wtk_string_t **path,int n,wtk_strbuf_t *buf)
{
	int i;
	int ret;
	int len;

	ret=wtk_rbin2_file_exist(rbin,fn,strlen(fn));
	if(ret==1){return fn;}
	len=strlen(fn);
	for(i=0;i<n;++i)
	{

#ifdef WIN32
		wtk_strbuf_reset(buf);
		wtk_strbuf_push(buf, path[i]->data, path[i]->len);
		wtk_strbuf_push_c(buf, DIR_SEP1);
		wtk_strbuf_push(buf, fn, len);
		ret = wtk_rbin2_file_exist(rbin, buf->data, buf->pos);
		//wtk_debug("[%.*s,ret=%d]\n", buf->pos, buf->data, ret);
		if (ret == 1)
		{
			wtk_strbuf_push_c(buf, 0);
			return buf->data;
		}
#endif
		//wtk_debug("[%.*s]\n",path[i]->len,path[i]->data);
		wtk_strbuf_reset(buf);
		wtk_strbuf_push(buf,path[i]->data,path[i]->len);
		wtk_strbuf_push_c(buf,DIR_SEP);
		wtk_strbuf_push(buf,fn,len);
		//wtk_strbuf_push_c(buf,0);
		ret=wtk_rbin2_file_exist(rbin,buf->data,buf->pos);
		//wtk_debug("[%s,ret=%d]\n",buf->data,ret);
		if(ret==1)
		{
			wtk_strbuf_push_c(buf,0);
			return buf->data;
		}
	}
	return NULL;
}

wtk_lua2_t* wtk_lua2_new(wtk_lua2_cfg_t *cfg,wtk_rbin2_t *rbin)
{
	wtk_lua2_t *lua=0;
	lua_State *state;
	wtk_string_t **strs;
	int i, ret = 0;
	wtk_strbuf_t *buf = 0;

	lua=(wtk_lua2_t*)wtk_malloc(sizeof(*lua));
	lua->rbin=rbin;
#ifdef WIN32
#else
	setlocale(LC_ALL,cfg->local_lang);
#endif
	state=luaL_newstate();
	luaL_openlibs(state);
#ifdef WIN32
#else
	wtk_lua_init_libs(state);
#endif
	if (cfg->libs)
	{
		char *fn;

		buf=wtk_strbuf_new(256,1);
		strs=cfg->libs->slot;
		for(i=0;i<cfg->libs->nslot;++i)
		{
			if(cfg->include_path)
			{
				if(rbin)
				{
					fn = wtk_lua_rbin_search_file(rbin, strs[i]->data, (wtk_string_t**)cfg->include_path->slot, cfg->include_path->nslot, buf);
				}
				else
				{
					fn=wtk_search_file(strs[i]->data,(wtk_string_t**)cfg->include_path->slot,cfg->include_path->nslot,buf);
				}
				if(!fn)
				{
					wtk_debug("[%s] not found.\n",strs[i]->data);
					goto end;
				}
			}
			else
			{
				fn=strs[i]->data;
			}
			ret=wtk_lua2_load_file(lua,state,fn);
			if(ret!=0){goto end;}
		}
	}
	if(cfg->fn)
	{
		ret=wtk_lua2_load_file(lua,state,cfg->fn);
		if(ret!=0){goto end;}
	}
	lua->state=state;
	lua->cfg=cfg;
end:
	//wtk_debug("lua=%p\n",lua);
	if(buf)
	{
		wtk_strbuf_delete(buf);
	}
	return lua;
}

int wtk_lua2_load(wtk_lua2_t *lua,char *data,int bytes)
{
	lua_State *state=lua->state;
	int ret;

	//wtk_debug("load [%s]\n",fn);
	//printf("[%.*s]\n",bytes,data);
	ret=luaL_loadbuffer(state,data,bytes,"__dummy");
	//wtk_debug("ret=%d;%s\n",ret,strs[i]->data);
	if(ret!=0)
	{
		wtk_debug("%s\n",lua_tostring(state,-1));
		goto end;
	}
	ret=lua_pcall(state,0,0,0);
	//wtk_debug("ret=%d\n",ret);
	if(ret!=0)
	{
		wtk_debug("%s\n",lua_tostring(state,-1));
		goto end;
	}
end:
	//wtk_debug("ret=%d\n",ret);
	return ret;
}


void wtk_lua2_delete(wtk_lua2_t *lua)
{
	lua_close(lua->state);
	wtk_free(lua);
}

int wtk_lua2_bytes(wtk_lua2_t *lua)
{
	lua_State *state=lua->state;

	return lua_gc(state,LUA_GCCOUNT,0)*1024+lua_gc(state,LUA_GCCOUNTB,0);
}

void wtk_lua2_gc(wtk_lua2_t *lua)
{
	lua_gc(lua->state,LUA_GCCOLLECT,0);
}

int wtk_lua2_process_number(wtk_lua2_t *lua,char *post_func,float arg,float *result)
{
	lua_State *state=lua->state;
	int ret;

	lua_getglobal(state,post_func);
	lua_pushnumber(state,arg);
	ret=lua_pcall(state,1,2,0);
	if(ret!=0){goto end;}
	ret=lua_gettop(state);
	if(ret<2){ret=-1;goto end;}
	ret=lua_isnumber(state,1);
	if(!ret){ret=-1;goto end;}
	ret=lua_tonumber(state,1);
	if(ret!=0){ret=-1;goto end;}
	ret=lua_isnumber(state,2);
	if(!ret){ret=-1;goto end;}
	*result=lua_tonumber(state,2);
	ret=0;
end:
	lua_settop(state,0);
	return ret;
}

int wtk_lua2_process_string(wtk_lua2_t *lua,char *post_func,char *arg,int arg_bytes,wtk_strbuf_t *result)
{
	lua_State *state=lua->state;
	size_t len;
	const char *p;
	int ret;

	//wtk_debug("check .............\n");
	//wtk_lua_gc(lua);
	lua_getglobal(state,post_func);
	lua_pushlstring(state,arg,arg_bytes);
	ret=lua_pcall(state,1,2,0);
	if(ret!=0){goto end;}
	ret=lua_gettop(state);
	if(ret<2){ret=-1;goto end;}
	ret=lua_isnumber(state,1);
	if(!ret){ret=-1;goto end;}
	ret=lua_tonumber(state,1);
	if(ret!=0){ret=-1;goto end;}
	ret=lua_isstring(state,2);
	if(!ret){ret=-1;goto end;}
	p=lua_tolstring(state,2,&len);
	//wtk_strbuf_reset(result);
	wtk_strbuf_push(result,p,len);
	ret=0;
end:
	if(ret!=0)
	{
		//wtk_debug("%s\n",lua_tostring(state,-1));
	}
	lua_settop(state,0);
	return ret;
}

int wtk_lua2_process_string2(wtk_lua2_t *lua,char *post_func,wtk_strbuf_t *result,...)
{
	va_list ap;
	lua_State *state=lua->state;
	size_t len;
	const char *p;
	wtk_string_t *v;
	int ret;
	int count=0;

	//wtk_debug("check .............\n");
	//wtk_lua_gc(lua);
	lua_getglobal(state,post_func);
    va_start(ap,result);
    while(1)
    {
        v=va_arg(ap,wtk_string_t*);
        if(!v){break;}
        //wtk_debug("%.*s\n",v->len,v->data);
        lua_pushlstring(state,v->data,v->len);
        ++count;
    }
    va_end(ap);
	ret=lua_pcall(state,count,2,0);
	if(ret!=0){goto end;}
	ret=lua_gettop(state);
	if(ret<2){ret=-1;goto end;}
	ret=lua_isnumber(state,1);
	if(!ret){ret=-1;goto end;}
	ret=lua_tonumber(state,1);
	if(ret!=0){ret=-1;goto end;}
	ret=lua_isstring(state,2);
	if(!ret){ret=-1;goto end;}
	p=lua_tolstring(state,2,&len);
	//wtk_strbuf_reset(result);
	wtk_strbuf_push(result,p,len);
	ret=0;
end:
	if(ret!=0)
	{
		wtk_debug("%s\n",lua_tostring(state,-1));
	}
	lua_settop(state,0);
	return ret;
}

int wtk_lua2_process_arg(wtk_lua2_t *lua,char *func,wtk_strbuf_t *result,...)
{
	va_list ap;
	lua_State *state=lua->state;
	size_t len;
	const char *p;
	wtk_lua2_arg_t *v;
	int ret;
	int count=0;
	int base;

	base=lua_gettop(state);
	//wtk_debug("lua process %s\n",func);
	//wtk_lua_gc(lua);
	wtk_strbuf_reset(result);
	lua_getglobal(state,func);
    va_start(ap,result);
    while(1)
    {
        v=va_arg(ap,wtk_lua2_arg_t*);
        if(!v){break;}
        //wtk_debug("%.*s\n",v->len,v->data);
        switch(v->type)
        {
        case WTK_LUA2_NUMBER:
        	lua_pushnumber(state,v->v.number);
        	break;
        case WTK_LUA2_STRING:
        	lua_pushlstring(state,v->v.str.data,v->v.str.len);
        	break;
        case WTK_LUA2_THS:
        	lua_pushlightuserdata(state,v->v.ths);
        	break;
        case WTK_LUA2_PUSH_VALUE:
        	v->v.push_value.push(state,v->v.push_value.ths);
        	break;
        }
        ++count;
    }
    va_end(ap);
	ret=lua_pcall(state,count,2,0);
	if(ret!=0){goto end;}
	ret=lua_gettop(state);
	if(ret<2)
	{
		wtk_debug("return must has 2 value\n");
		ret=-1;goto end;
	}
	ret=lua_isnumber(state,base+1);
	if(!ret)
	{
		int t;

		t=lua_type(state,1);
		wtk_debug("%s: return is not number %s num=%d\n",func,lua_typename(state,t),lua_gettop(state));
		ret=-1;goto end;
	}
	ret=lua_tonumber(state,base+1);
	if(ret!=0)
	{
		wtk_debug("%s return is not number\n",func);
		ret=-1;goto end;
	}
	ret=lua_isstring(state,base+2);
	if(!ret)
	{
		wtk_debug("%s return is not string\n",func);
		ret=-1;goto end;
	}
	p=lua_tolstring(state,base+2,&len);
	wtk_strbuf_reset(result);
	wtk_strbuf_push(result,p,len);
	//wtk_debug("[%.*s]\n",(int)len,p);
	ret=0;
end:
	if(ret!=0)
	{
		const char *s;

		s=lua_tostring(state,-1);
		if(s)
		{
			wtk_debug("%s:%s\n",func,s);
			//exit(0);
		}
	}
	//wtk_debug("ret=%d %s\n",ret,func);
	//wtk_debug("[%.*s]\n",result->pos,result->data);
	lua_settop(state,0);
	//wtk_debug("leave lua detph=%d ret=%d\n",depth,ret);
	return ret;
}
