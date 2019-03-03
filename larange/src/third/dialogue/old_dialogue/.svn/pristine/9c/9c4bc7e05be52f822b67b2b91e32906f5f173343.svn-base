#include "wtk_semlua.h" 
#include "wtk/core/wtk_fkv.h" 
#include "wtk/semdlg/wtk_semdlg.h"
#include "wtk/semdlg/semfld/wtk_semfld.h"
#include "qtk/https/qtk_https.h"
#ifdef __ANDROID__
#else
#ifdef __arm__
#include "wiringPi.h"
#include "softPwm.h"
#define USE_PI
#endif
#endif


void wtk_gpio_wiringPiSetup()
{
	//wtk_debug("setup\n");
#ifdef USE_PI
	wiringPiSetup();
#endif
}

void wtk_gpio_pinMode(int pin,int mode)
{
	//wtk_debug("set %d %d\n",pin,mode);
#ifdef USE_PI
	pinMode(pin, mode);
#endif
}

void wtk_gpio_digitalWrite(int pin,int v)
{
	//wtk_debug("write %d %d\n",pin,v);
#ifdef USE_PI
	digitalWrite(pin, v);
#endif
}

int wtk_gpio_digitalRead(int pin)
{
	int v;

	v=0;
#ifdef USE_PI
	v=digitalRead(pin);
#endif
	//wtk_debug("read %d %d\n",pin,v);
	return v;
}

void wtk_gpio_softPwmWrite(int pin,int v)
{
	//wtk_debug("write %d %d\n",pin,v);
#ifdef USE_PI
	softPwmWrite(pin,v);
#endif
}

void wtk_gpio_softPwmCreate(int pin,int v,int type)
{
	//wtk_debug("write %d %d\n",pin,v);
#ifdef USE_PI
	softPwmCreate(pin,v,type);
#endif
}

void wtk_gpio_pullUpDnControl(int pin,int v)
{
	//wtk_debug("write %d %d\n",pin,v);
#ifdef USE_PI
	pullUpDnControl(pin,v);
#endif
}

int wtk_lua_gpio_wiringPiSetup(lua_State *l)
{
	wtk_gpio_wiringPiSetup();
	return 0;
}

int wtk_lua_gpio_pinmode(lua_State *l)
{
	int i,pin,v;

	i=1;
	if(!lua_isnumber(l,i)){goto end;}
	pin=lua_tonumber(l,i);
	++i;
	if(!lua_isnumber(l,i)){goto end;}
	v=lua_tonumber(l,i);
	wtk_gpio_pinMode(pin,v);
end:
	return 0;
}

int wtk_lua_gpio_digitalWrite(lua_State *l)
{
	int i,pin,v;

	i=1;
	if(!lua_isnumber(l,i)){goto end;}
	pin=lua_tonumber(l,i);
	++i;
	if(!lua_isnumber(l,i)){goto end;}
	v=lua_tonumber(l,i);
	wtk_gpio_digitalWrite(pin,v);
end:
	return 0;
}

int wtk_lua_gpio_digitalRead(lua_State *l)
{
	int i,pin,v;
	int cnt=0;

	i=1;
	if(!lua_isnumber(l,i)){goto end;}
	pin=lua_tonumber(l,i);
	v=wtk_gpio_digitalRead(pin);
	lua_pushnumber(l,v);
	cnt=1;
end:
	return cnt;
}

int wtk_lua_gpio_softPwmWrite(lua_State *l)
{
	int i,pin,v;

	i=1;
	if(!lua_isnumber(l,i)){goto end;}
	pin=lua_tonumber(l,i);
	++i;
	if(!lua_isnumber(l,i)){goto end;}
	v=lua_tonumber(l,i);
	wtk_gpio_softPwmWrite(pin,v);
end:
	return 0;
}

int wtk_lua_gpio_softPwmCreate(lua_State *l)
{
	int i,pin,v,t;

	i=1;
	if(!lua_isnumber(l,i)){goto end;}
	pin=lua_tonumber(l,i);
	++i;
	if(!lua_isnumber(l,i)){goto end;}
	v=lua_tonumber(l,i);
	++i;
	if(!lua_isnumber(l,i)){goto end;}
	t=lua_tonumber(l,i);
	wtk_gpio_softPwmCreate(pin,v,t);
end:
	return 0;
}

int wtk_lua_gpio_pullUpDnControl(lua_State *l)
{
	int i,pin,v;

	//wtk_debug("====================\n");
	i=1;
	if(!lua_isnumber(l,i))
	{
		wtk_debug("not number\n");
		goto end;
	}
	pin=lua_tonumber(l,i);
	++i;
	if(!lua_isnumber(l,i))
	{
		wtk_debug("not number\n");
		goto end;
	}
	v=lua_tonumber(l,i);
	wtk_gpio_pullUpDnControl(pin,v);
end:
	return 0;
}

int wtk_lua_semdlg_set_output(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t v;
	size_t len;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	//wtk_debug("[%.*s]\n",v.len,v.data);
	wtk_semdlg_set_output(sem,v.data,v.len);
end:
	return 0;
}

int wtk_lua_semfld_set_output(lua_State *l)
{
	wtk_semfld_t *sem;
	wtk_string_t v;
	size_t len;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semfld_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	//wtk_debug("[%.*s]\n",v.len,v.data);
	wtk_semfld_set_output(sem,v.data,v.len);
end:
	return 0;
}

int wtk_lua_semfld_get_output(lua_State *l)
{
	wtk_semfld_t *sem;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semfld_t*)lua_touserdata(l,i);
	if(sem->output)
	{
		lua_pushlstring(l,sem->output->data,sem->output->len);
		cnt=1;
	}
end:
	return cnt;
}

int wtk_lua_semfld_get_slot(lua_State *l)
{
	wtk_semfld_t *sem;
	wtk_string_t v;
	size_t len;
	int i;
	int cnt=0;
	wtk_string_t *p;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semfld_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	//wtk_debug("[%.*s]\n",v.len,v.data);
	p=wtk_semfld_get(sem,v.data,v.len);
	if(!p){goto end;}
	lua_pushlstring(l,p->data,p->len);
	cnt=1;
end:
	return cnt;
}

int wtk_lua_semfld_del_slot(lua_State *l)
{
	wtk_semfld_t *sem;
	wtk_string_t v;
	size_t len;
	int i;

	//wtk_debug("del slot\n");
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semfld_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	//wtk_debug("[%.*s]\n",v.len,v.data);
	wtk_semslot_del(sem->slot,v.data,v.len);
	//wtk_semslot_print(sem->slot);
end:
	return 0;
}

int wtk_lua_semfld_del_slot2(lua_State *l)
{
	wtk_semfld_t *sem;
	wtk_string_t v;
	size_t len;
	int i;

	//wtk_debug("check. ...\n");
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semfld_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	//wtk_debug("[%.*s]\n",v.len,v.data);
	wtk_semfld_del(sem,v.data,v.len);
end:
	//exit(0);
	return 0;
}

int wtk_lua_semfld_process_nlg(lua_State *l)
{
	wtk_semfld_t *fld;
	wtk_string_t k;
	size_t len;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	wtk_semfld_process_nlg(fld,k.data,k.len);
end:
	return 0;
}

int wtk_lua_semfld_process_nlg2(lua_State *l)
{
	wtk_semfld_t *fld;
	wtk_string_t k;
	size_t len;
	int i;
	int ret=-1;

	//wtk_debug("===========================>\n");
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	//wtk_debug("[%.*s]\n",k.len,k.data);
	ret=wtk_semfld_process_nlg2(fld,k.data,k.len);
end:
	lua_pushnumber(l,ret);
	return 1;
}

int wtk_lua_semfld_process_nlg3(lua_State *l)
{
	wtk_semfld_t *fld;
	wtk_string_t k;
	size_t len;
	int i;
	int ret=-1;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	ret=wtk_semfld_process_nlg2(fld,k.data,k.len);
	if(ret!=0){goto end;}
	//wtk_debug("output=%p\n",fld->output);
	if(fld->output)
	{
		lua_pushlstring(l,fld->output->data,fld->output->len);
		fld->output=NULL;
		++cnt;
	}
end:
	return cnt;
}

int wtk_lua_semfld_get_owl(lua_State *l)
{
	wtk_semfld_t *fld;
	wtk_string_t *v;
	wtk_string_t k;
	size_t len;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	if(!fld->cfg->owl){goto end;}
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	v=wtk_owl_tree_expr_value(fld->cfg->owl,k.data,k.len);
	if(v)
	{
		lua_pushlstring(l,v->data,v->len);
		++cnt;
	}
end:
	return cnt;
}


int wtk_lua_semfld_process_owl(lua_State *l)
{
	wtk_semfld_t *fld;
	wtk_act_t *act;
	wtk_string_t *v;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isuserdata(l,i)){goto end;}
	act=(wtk_act_t*)lua_touserdata(l,i);
	v=wtk_semfld_process_owl(fld,act);
	if(v)
	{
		lua_pushlstring(l,v->data,v->len);
		++cnt;
	}
end:
	return cnt;
}

//int wtk_lua_semfld_process_lex(lua_State *l)
//{
//	wtk_semfld_t *fld;
//	wtk_string_t k;
//	size_t len;
//	int i;
//	int cnt=0;
//
//	i=1;
//	if(!lua_isuserdata(l,i)){goto end;}
//	fld=(wtk_semfld_t*)lua_touserdata(l,i);
//	++i;
//	if(!lua_isstring(l,i)){goto end;}
//	k.data=(char*)lua_tolstring(l,i,&len);
//	k.len=len;
//	//wtk_debug("[%.*s]\n",k.len,k.data);
//	//wtk_semfld_process_lex(fld,k.data,k.len);
//end:
//	return cnt;
//}

int wtk_lua_semfld_get_dn(lua_State *l)
{
	wtk_semfld_t *fld;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	if(fld->cfg->dn)
	{
		lua_pushstring(l,fld->cfg->dn);
		cnt=1;
	}
end:
	return cnt;
}

int wtk_lua_semfld_set_fst_state(lua_State *l)
{
	wtk_semfld_t *fld;
	wtk_string_t k;
	size_t len;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	//wtk_debug("[%.*s]\n",k.len,k.data);
	wtk_semfld_set_fst_state(fld,k.data,k.len);
end:
	return 0;
}

int wtk_lua_semfld_get_fst_state(lua_State *l)
{
	wtk_semfld_t *fld;
	//wtk_string_t k;
	//size_t len;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	if(fld->fst && fld->fst->cur_state)
	{
		lua_pushlstring(l,fld->fst->cur_state->name->data,fld->fst->cur_state->name->len);
		cnt=1;
	}
end:
	return cnt;
}

int wtk_lua_semfld_remove_from_ctx(lua_State *l)
{
	wtk_semfld_t *fld;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	fld->want_add_hist=0;
	fld->dlg->last_fld=NULL;
	//fld->dlg->cur_fld=NULL;
end:
	return 0;
}

int wtk_lua_semfld_get_data_fn(lua_State *l)
{
	wtk_semfld_t *fld;
	wtk_string_t k,v;
	size_t len;
	int i;
	wtk_strbuf_t *buf;
	int n;
	int pos;
	int cnt=0;

	n=lua_gettop(l);
	//wtk_debug("n=%d\n",n);
	buf=wtk_strbuf_new(256,1);
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	if(!fld->cfg->dat_dn){goto end;}
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	wtk_strbuf_push(buf,fld->cfg->dat_dn,strlen(fld->cfg->dat_dn));
	wtk_strbuf_push_s(buf,"/");
	wtk_strbuf_push(buf,k.data,k.len);
	wtk_strbuf_push_s(buf,".");
	pos=buf->pos;
	for(++i;i<=n;++i)
	{
		if(!lua_isstring(l,i)){goto end;}
		v.data=(char*)lua_tolstring(l,i,&len);
		v.len=len;
		//wtk_debug("[%.*s]\n",v.len,v.data);
		buf->pos=pos;
		wtk_strbuf_push(buf,v.data,v.len);
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		wtk_strbuf_push_c(buf,0);
		if(wtk_file_exist(buf->data)==0)
		{
			wtk_strbuf_t *xbuf;

			xbuf=wtk_strbuf_new(256,1);
			wtk_real_fn(buf->data,buf->pos-1,xbuf,'/');
			lua_pushlstring(l,xbuf->data,xbuf->pos);
			wtk_strbuf_delete(xbuf);
			cnt=1;
			goto end;
		}
	}
end:
	//exit(0);
	wtk_strbuf_delete(buf);
	return cnt;
}

int wtk_lua_semfld_set_slot(lua_State *l)
{
	wtk_semfld_t *fld;
	wtk_string_t k,v;
	size_t len;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	wtk_semslot_set(fld->slot,k.data,k.len,v.data,v.len);
end:
	return 0;
}

int wtk_lua_semfld_set_slot2(lua_State *l)
{
	wtk_semfld_t *fld;
	wtk_string_t k,v;
	size_t len;
	int i;
	int cnt;
	int usr_history;

	cnt=lua_gettop(l);
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	if(cnt>3)
	{
		++i;
		if(!lua_isnumber(l,i)){goto end;}
		usr_history=lua_tonumber(l,i);
	}else
	{
		usr_history=0;
	}
	wtk_semfld_set_str(fld,k.data,k.len,v.data,v.len,usr_history);
end:
	return 0;
}

int wtk_lua_semfld_get_ask_slot(lua_State *l)
{
	wtk_semfld_t *sem;
	int i;
	int cnt=0;
	wtk_string_t p;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semfld_t*)lua_touserdata(l,i);
	//wtk_debug("[%.*s]\n",v.len,v.data);
	p=wtk_semfld_get_ask_slot(sem);
	if(p.len<=0){goto end;}
	lua_pushlstring(l,p.data,p.len);
	cnt=1;
end:
	return cnt;
}

int wtk_lua_semfld_reset_slot(lua_State *l)
{
	wtk_semfld_t *sem;
	int i;
	//wtk_string_t p;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semfld_t*)lua_touserdata(l,i);
	//wtk_debug("[%.*s]\n",v.len,v.data);
	wtk_semslot_reset(sem->slot);
	wtk_semfld_set_ask_slot(sem,0,0,0,0);
end:
	return 0;
}

int wtk_lua_semfld_get_ask_slot_value(lua_State *l)
{
	wtk_semfld_t *sem;
	int i;
	int cnt=0;
	wtk_string_t p;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semfld_t*)lua_touserdata(l,i);
	//wtk_debug("[%.*s]\n",v.len,v.data);
	p=wtk_semfld_get_ask_slot_value(sem);
	if(p.len<=0){goto end;}
	lua_pushlstring(l,p.data,p.len);
	cnt=1;
end:
	return cnt;
}

int wtk_lua_semfld_set_ask_slot(lua_State *l)
{
	wtk_semfld_t *fld;
	wtk_string_t k;
	size_t len;
	int i;
	int cnt;
	wtk_string_t v;

	wtk_string_set(&(v),0,0);
	cnt=lua_gettop(l);
	//wtk_debug("cnt=%d\n",cnt);
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	if(cnt>2)
	{
		++i;
		if(!lua_isstring(l,i)){goto end;}
		v.data=(char*)lua_tolstring(l,i,&len);
		v.len=len;
	}
	wtk_semfld_set_ask_slot(fld,k.data,k.len,v.data,v.len);
end:
	//exit(0);
	return 0;
}

int wtk_lua_semfld_get_ask_deny_func(lua_State *l)
{
	wtk_semfld_t *sem;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semfld_t*)lua_touserdata(l,i);
	if(sem->ask_deny_lua_func->pos>0)
	{
		++cnt;
		lua_pushlstring(l,sem->ask_deny_lua_func->data,sem->ask_deny_lua_func->pos);
	}
	if(sem->ask_deny_lua_str->pos>0)
	{
		++cnt;
		lua_pushlstring(l,sem->ask_deny_lua_str->data,sem->ask_deny_lua_str->pos);
	}
end:
	return cnt;
}


int wtk_lua_semfld_set_ask_deny_func(lua_State *l)
{
	wtk_semfld_t *fld;
	wtk_string_t k;
	size_t len;
	int i;
	int cnt;
	wtk_string_t v;

	wtk_string_set(&(v),0,0);
	cnt=lua_gettop(l);
	//wtk_debug("cnt=%d\n",cnt);
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	if(cnt>2)
	{
		++i;
		if(!lua_isstring(l,i)){goto end;}
		v.data=(char*)lua_tolstring(l,i,&len);
		v.len=len;
	}
	wtk_semfld_set_ask_deny_func(fld,k.data,k.len,v.data,v.len);
end:
	//exit(0);
	return 0;
}


int wtk_lua_semfld_set_end(lua_State *l)
{
	wtk_semfld_t *fld;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	wtk_semfld_set_end(fld);
end:
	//wtk_debug("return 0\n");
	return 0;
}

//int wtk_lua_semfld_get_owlfst(lua_State *l)
//{
//	wtk_semfld_t *fld;
//	int i;
//	int cnt=0;
//
//	i=1;
//	if(!lua_isuserdata(l,i)){goto end;}
//	fld=(wtk_semfld_t*)lua_touserdata(l,i);
//	if(fld->owlfst)
//	{
//		lua_pushlightuserdata(l,fld->owlfst);
//		cnt=1;
//	}
//end:
//	return cnt;
//}

int wtk_lua_semfld_goto_state(lua_State *l)
{
	wtk_semfld_t *fld;
	wtk_string_t v;
	size_t len;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	wtk_semfld_goto_state(fld,v.data,v.len);
end:
	return 0;
}


int wtk_lua_semdlg_get_fld(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_semfld_t *fld;
	wtk_string_t v;
	size_t len;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	//wtk_debug("[%.*s]\n",v.len,v.data);
	fld=wtk_semdlg_get_fld(sem,v.data,v.len);
	if(!fld){goto end;}
	lua_pushlightuserdata(l,fld);
	cnt=1;
end:
	return cnt;
}

int wtk_lua_semdlg_get_last_fld(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_semfld_t *fld;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	fld=sem->last_fld;
	if(fld)
	{
		lua_pushlightuserdata(l,fld);
		cnt=1;
	}
end:
	return cnt;
}


int wtk_lua_semdlg_set_next_fld(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_semfld_t *fld;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isuserdata(l,i))
	{
		if(lua_isnil(l,i))
		{
			wtk_semdlg_set_next_fld(sem,NULL);
			//sem->cur_fld=NULL;
		}
		goto end;
	}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	wtk_semdlg_set_next_fld(sem,fld);
end:
	return 0;
}

int wtk_lua_semdlg_set_last_fld(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_semfld_t *fld;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isuserdata(l,i))
	{
		if(lua_isnil(l,i))
		{
			wtk_semdlg_set_last_fld(sem,NULL);
			//sem->cur_fld=NULL;
		}
		goto end;
	}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	wtk_semdlg_set_last_fld(sem,fld);
end:
	return 0;
}

int wtk_lua_semdlg_quit_fld(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_semfld_t *fld;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isuserdata(l,i))
	{
		if(lua_isnil(l,i))
		{
			wtk_semdlg_set_last_fld(sem,NULL);
			//sem->cur_fld=NULL;
		}
		goto end;
	}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	wtk_semdlg_semdlg_quit_fld(sem,fld);
end:
	return 0;
}

int wtk_lua_semdlg_remove_history(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_semfld_t *fld;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isuserdata(l,i)){goto end;}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	wtk_semdlg_remove_history(sem,fld);
end:
	return 0;
}

int wtk_lua_semdlg_get_input(lua_State *l)
{
	wtk_semdlg_t *sem;
	int cnt=0;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	if(!sem->input){goto end;}
	lua_pushlstring(l,sem->input->data,sem->input->len);
	cnt=1;
end:
	return cnt;
}

int wtk_lua_semdlg_has_process_domain(lua_State *l)
{
	wtk_semdlg_t *sem;
	int cnt=0;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	lua_pushnumber(l,sem->domained);
	cnt=1;
end:
	return cnt;
}

int wtk_lua_semdlg_process(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t v;
	size_t len;
	int i;
	wtk_semfld_t *fld;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	fld=sem->last_fld;
	//wtk_debug("last=%p\n",sem->last_fld);
	sem->last_fld=NULL;
	sem->cur_fld=NULL;
	wtk_semdlg_process(sem,v.data,v.len);
	sem->last_fld=fld;
	//wtk_debug("last=%p\n",sem->last_fld);
end:
	return 0;
}

wtk_json_item_t* wtk_lua_expr_to_json(wtk_json_t *json,lua_State *l,int idx)
{
	wtk_json_item_t *item,*vi;
	int type;
	int v;
	float f;
	size_t len;
	char *s;
	int i;

	//wtk_debug("json=%p\n",json);
	item=NULL;
	type=lua_type(l,idx);
	//wtk_debug("type=%d idx=%d\n",type,idx);
	switch(type)
	{
	case LUA_TNIL:
		item=wtk_json_new_item(json,WTK_JSON_NULL);
		break;
	case LUA_TBOOLEAN:
		v=lua_toboolean(l,idx);
		item=wtk_json_new_item(json,v>0?WTK_JSON_TRUE:WTK_JSON_FALSE);
		break;
	case LUA_TNUMBER:
		f=lua_tonumber(l,idx);
		//wtk_debug("f=%f\n",f);
		item=wtk_json_new_number(json,f);
		//wtk_json_item_print3(item);
		//wtk_debug("v=%f\n",item->v.number);
		break;
	case LUA_TSTRING:
		s=(char*)lua_tolstring(l,idx,&len);
		if(s && len>0)
		{
			item=wtk_json_new_string(json,s,len);
		}else
		{
			item=wtk_json_new_string(json,"",0);
		}
		break;
	case LUA_TTABLE:
		v=lua_objlen(l,idx);
		if(v==0)
		{
			item=wtk_json_new_object(json);
			v=lua_gettop(l);
			lua_pushnil(l);
			while(lua_next(l,v)!=0)
			{
				s=(char*)lua_tolstring(l,-2,&len);
				if(s)
				{
					//printf("%s\n",s);
					vi=wtk_lua_expr_to_json(json,l,-1);
					wtk_json_obj_add_item2(json,item,s,len,vi);
				}
				lua_pop(l,1);
			}
			//lua_pop(l,1);
			//exit(0);
		}else
		{
			item=wtk_json_new_array(json);
			for(i=1;i<=v;++i)
			{
				//wtk_debug("i=%d\n",i);
				lua_rawgeti(l,idx,i);
				vi=wtk_lua_expr_to_json(json,l,-1);
				if(vi)
				{
					wtk_json_array_add_item(json,item,vi);
					//wtk_debug("i=%d\n",i);
					//wtk_json_item_print3(vi);
					//lua_pop(l,1);
				}
				lua_pop(l,1);
			}
			//lua_pop(l,1);
		}
		break;
	default:
		break;
	}
//	if(item)
//	{
//		wtk_json_item_print3(item);
//	}
	return item;
}

int wtk_lua_semdlg_set_ext_json(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t v;
	wtk_json_item_t *item;
	size_t len;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	//wtk_debug("i=%d\n",i);
	item=wtk_lua_expr_to_json(sem->json,l,++i);
	if(item)
	{
		//wtk_json_item_print3(item);
		wtk_semdlg_set_ext_json(sem,v.data,v.len,item);
	}
end:
	//exit(0);
	return 0;
}

int wtk_lua_semdlg_exe(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_json_item_t *item;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	//wtk_debug("i=%d\n",i);
	item=wtk_lua_expr_to_json(sem->json,l,++i);
	if(item)
	{
		//wtk_json_item_print3(item);
		wtk_semdlg_exe(sem,item);
		if(sem->exe_ret.len>0)
		{
			lua_pushlstring(l,sem->exe_ret.data,sem->exe_ret.len);
			cnt=1;
		}
	}
end:
	//exit(0);
	return cnt;
}

int wtk_lua_semdlg_get_env(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t v;
	size_t len;
	int i;
	int cnt=0;
	wtk_string_t *xv;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	if(!sem->env_parser || !sem->env_parser->main)
	{
		goto end;
	}
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	//wtk_debug("ret=x\n");
	xv=wtk_local_cfg_find_string(sem->env_parser->main,v.data,v.len);
	if(xv)
	{
		lua_pushlstring(l,xv->data,xv->len);
		cnt=1;
	}
end:
	//wtk_debug("ret=0\n");
	return cnt;
}


int wtk_lua_semdlg_syn(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t v;
	size_t len;
	int i;
	int syn;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	++i;
	if(!lua_isnumber(l,i)){goto end;}
	syn=lua_tonumber(l,i);
	//wtk_debug("ret=x\n");
	wtk_semdlg_syn(sem,v.data,v.len,syn);
end:
	//wtk_debug("ret=0\n");
	return 0;
}

int wtk_lua_semdlg_play_file(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t v;
	size_t len;
	int i;
	int syn;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	++i;
	if(!lua_isnumber(l,i)){goto end;}
	syn=lua_tonumber(l,i);
	wtk_semdlg_play_file(sem,v.data,v.len,syn);
end:
	return 0;
}

int wtk_lua_semdlg_set_rec_grammar(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t v;
	size_t len;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	wtk_semdlg_set_rec_grammar(sem,v.data,v.len);
end:
	return 0;
}

int wtk_lua_semdlg_get_ext_input(lua_State *l)
{
	wtk_semdlg_t *sem;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	if(sem->ext_item)
	{
		wtk_lua_push_json(l,sem->ext_item);
		cnt=1;
	}
end:
	//exit(0);
	return cnt;
}

int wtk_lua_semdlg_get_conf(lua_State *l)
{
	wtk_semdlg_t *sem;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	lua_pushnumber(l,sem->conf);
	cnt=1;
end:
	//exit(0);
	return cnt;
}

int wtk_lua_semdlg_get_vadtime(lua_State *l)
{
	wtk_semdlg_t *sem;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	lua_pushnumber(l,sem->vad_time);
	cnt=1;
end:
	//exit(0);
	return cnt;
}

int wtk_lua_semdlg_kv_get_dat(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t k;
	wtk_string_t v;
	size_t len;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	v=wtk_jsonkv_get_dat(sem->jsonkv,k.data,k.len);
	if(v.len>0)
	{
		lua_pushlstring(l,v.data,v.len);
		cnt=1;
	}
end:
	return cnt;
}

int wtk_lua_semdlg_datkv_get_random(lua_State *l)
{
    wtk_semdlg_t *sem;
    wtk_fkv_t *fkv;
	wtk_string_t v;
    uint32_t rv1;
	size_t len;
	int i;
	int cnt=0;
    int random_cnt = 0;
	wtk_string_t db;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	if(!sem->kgkv){goto end;}

	++i;
	if(!lua_isstring(l,i)){goto end;}
	db.data=(char*)lua_tolstring(l,i,&len);
	db.len=len;

	while (random_cnt < 1000) {
		fkv=(wtk_fkv_t*)wtk_str_hash_find(sem->kgkv->hash,db.data,db.len);
		if(!fkv) {
            random_cnt++;
            continue;
        }

		rv1=rand()%fkv->nid;
		if(fkv->idx[rv1] < 1) {
            random_cnt++;
            continue;
        }

		v=wtk_fkv_get_item_str(fkv,fkv->idx[rv1]+fkv->data_offset,NULL,0);
		if(v.len>0){
			lua_pushlstring(l,v.data,v.len);
			cnt=1;
			break;
		}
	}

end:
    return cnt;
}

int wtk_lua_semdlg_datkv_get(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t k;
	wtk_string_t v;
	size_t len;
	int i;
	int cnt=0;
	int n;
	wtk_string_t db;

	n=lua_gettop(l);
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	if(!sem->kgkv){goto end;}
	if(n==2)
	{
		++i;
		if(!lua_isstring(l,i)){goto end;}
		k.data=(char*)lua_tolstring(l,i,&len);
		k.len=len;
		//wtk_debug("[%.*s] n=%d\n",k.len,k.data,n);
		v=wtk_kgkv_get(sem->kgkv,NULL,&k);
	}else
	{
		++i;
		if(!lua_isstring(l,i)){goto end;}
		db.data=(char*)lua_tolstring(l,i,&len);
		db.len=len;
		++i;
		if(!lua_isstring(l,i)){goto end;}
		k.data=(char*)lua_tolstring(l,i,&len);
		k.len=len;
		//wtk_debug("[%.*s] n=%d\n",k.len,k.data,n);
		v=wtk_kgkv_get(sem->kgkv,&db,&k);
	}
	//exit(0);
	//v=wtk_fkv_get_str2(sem->fkv,k.data,k.len);
	if(v.len>0)
	{
		lua_pushlstring(l,v.data,v.len);
		cnt=1;
	}
end:
	return cnt;
}


int wtk_lua_semdlg_kv_get(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t k;
	wtk_string_t a;
	wtk_string_t v;
	size_t len;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	a.data=(char*)lua_tolstring(l,i,&len);
	a.len=len;
	v=wtk_jsonkv_get(sem->jsonkv,k.data,k.len,a.data,a.len);
	lua_pushlstring(l,v.data,v.len);
	cnt=1;
end:
	return cnt;
}


int wtk_lua_semdlg_get_last_input(lua_State *l)
{
	wtk_semdlg_t *sem;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	//wtk_debug("[%.*s]\n",sem->last_input->pos,sem->last_input->data);
	lua_pushlstring(l,sem->last_input->data,sem->last_input->pos);
	cnt=1;
end:
	return cnt;
}

int wtk_lua_semdlg_get_last_output(lua_State *l)
{
	wtk_semdlg_t *sem;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	//wtk_debug("[%.*s]\n",sem->last_input->pos,sem->last_input->data);
	lua_pushlstring(l,sem->last_output->data,sem->last_output->pos);
	cnt=1;
end:
	return cnt;
}


int wtk_lua_semdlg_last_is_faq(lua_State *l)
{
	wtk_semdlg_t *sem;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	//wtk_debug("[%.*s]\n",sem->last_input->pos,sem->last_input->data);
	lua_pushnumber(l,sem->use_faq);
	cnt=1;
end:
	return cnt;
}


int wtk_lua_semdlg_add_faq(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t k;
	wtk_string_t a;
	size_t len;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	if(!sem->faq){goto end;}
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	a.data=(char*)lua_tolstring(l,i,&len);
	a.len=len;
	wtk_faq_add(sem->faq,k.data,k.len,a.data,a.len);
	cnt=0;
end:
	return cnt;
}

int wtk_lua_semdlg_get_faq(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t k;
	int depth;
	size_t len;
	int i;
	int cnt=0;
	int nx;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	if(!sem->faq){goto end;}
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	++i;
	if(!lua_isnumber(l,i))
	{
		//goto end;
		depth=1;
	}else
	{
		depth=lua_tonumber(l,i);
	}
	nx=sem->faq->v.vecfaq->cfg->ndat;
	depth=min(nx,depth);
	sem->faq->v.vecfaq->cfg->ndat=depth;
	k=wtk_faq_get(sem->faq,k.data,k.len);
	sem->faq->v.vecfaq->cfg->ndat=nx;
	if(k.len>0)
	{
		lua_pushlstring(l,k.data,k.len);
		cnt=1;
	}
end:
	return cnt;
}

int wtk_lua_semdlg_get_faq2(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t s,k;
	size_t len;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	if(!sem->faq){goto end;}
	++i;
	if(!lua_isstring(l,i)){goto end;}
	s.data=(char*)lua_tolstring(l,i,&len);
	s.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	//wtk_debug("[%.*s]=[%.*s]\n",s.len,s.data,k.len,k.data);
	k=wtk_faq_get2(sem->faq,s.data,s.len,k.data,k.len);
	//wtk_debug("[%.*s]\n",k.len,k.data);
	if(k.len>0)
	{
		lua_pushlstring(l,k.data,k.len);
		cnt=1;
	}
end:
	return cnt;
}

int wtk_lua_semdlg_kv_set_dat(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t k;
	wtk_string_t v;
	size_t len;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	wtk_jsonkv_set_dat(sem->jsonkv,k.data,k.len,v.data,v.len);
end:
	return 0;
}

int wtk_lua_semdlg_kv_set(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t k;
	wtk_string_t a;
	wtk_string_t v;
	size_t len;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	a.data=(char*)lua_tolstring(l,i,&len);
	a.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	wtk_jsonkv_set(sem->jsonkv,k.data,k.len,a.data,a.len,v.data,v.len);
end:
	return 0;
}


int wtk_lua_semdlg_get_like(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t k;
	int n;
	size_t len;
	int i;
	int cnt=0;
	wtk_robin_t *rb;
	double thresh;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	if(!sem->wrdvec){goto end;}
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	++i;
	if(!lua_isnumber(l,i)){goto end;}
	n=lua_tonumber(l,i);
	++i;
	if(!lua_isnumber(l,i))
	{
		thresh=0.8;
	}else
	{
		thresh=lua_tonumber(l,i);
	}
	rb=wtk_wrdvec_find_best_like_word(sem->wrdvec,k.data,k.len,n,thresh);
	if(rb->used>0)
	{
		wtk_string_t *str;

		lua_newtable(l);
		for(i=0;i<rb->used;++i)
		{
			str=wtk_robin_at(rb,i);
			//lua_pushnumber(l,i+1);
			//wtk_debug("v[%d]=%.*s\n",i,str->len,str->data);
			lua_pushlstring(l,str->data,str->len);
			//lua_pushstring(l,"hello");
			//lua_pushnumber(l,i+1);
			//wtk_debug("set  1244 end\n");
			//lua_settable(l,-3);
			lua_rawseti(l, -2, i+1);
			//lua_rawset(l,-3);
			//lua_rawseti(l,1,i+1);
		}
		//wtk_debug("set  123333 end\n");
		cnt=1;
	}
	wtk_robin_delete(rb);
end:
	//wtk_debug("=============> cnt=%d\n",cnt);
	return cnt;
}

int wtk_lua_semdlg_get_wrdvec_like(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t v1,v2;
	size_t len;
	float thresh;
	int cnt=0;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	if(!sem->wrdvec){goto end;}
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v1.data=(char*)lua_tolstring(l,i,&len);
	v1.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v2.data=(char*)lua_tolstring(l,i,&len);
	v2.len=len;
	thresh=wtk_wrdvec_like(sem->wrdvec,v1.data,v1.len,v2.data,v2.len);
	if(thresh)
	{
		cnt=1;
		lua_pushnumber(l,thresh);
	}
end:
	return cnt;
}

int wtk_lua_semdlg_set_has_post_audio(lua_State *l)
{
	wtk_semdlg_t *sem;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	sem->has_post_audio=1;
end:
	return 0;
}

#include "wtk/http/misc/httpc/wtk_httpc.h"

int wtk_lua_http_get(lua_State *l)
{
	wtk_string_t host;
	wtk_string_t port;
	wtk_string_t url;
	wtk_httpc_cfg_t cfg;
	wtk_httpc_t *hc;
	int i;
	int cnt=0;
	size_t len;
	int timeout;

	i=1;
	if(!lua_isstring(l,i)){goto end;}
	host.data=(char*)lua_tolstring(l,i,&len);
	host.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	port.data=(char*)lua_tolstring(l,i,&len);
	port.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	url.data=(char*)lua_tolstring(l,i,&len);
	url.len=len;
	++i;
	if(!lua_isnumber(l,i)){goto end;}
	timeout=lua_tonumber(l,i);

	wtk_httpc_cfg_init(&(cfg));
	cfg.ip=host;
	cfg.port=port;
	cfg.url=url;
	cfg.timeout=timeout;
	cfg.use_1_1=0;
	hc=wtk_httpc_new(&(cfg));
	wtk_httpc_req_s(hc,"");
	//wtk_http_response_print(hc->req->response);
	if(hc->req->response->body->pos>0)
	{
		lua_pushlstring(l,hc->req->response->body->data,hc->req->response->body->pos);
		cnt=1;
	}
	wtk_httpc_delete(hc);
end:
	return cnt;
}

int qtk_lua_https_get(lua_State *l)
{
	int i;
	int cnt=0;
	size_t len;
	//int timeout;
	qtk_https_t *https = NULL;

	https = qtk_https_new();

	i=1;
	if(!lua_isstring(l,i)){goto end;}
	https->host.data=(char*)lua_tolstring(l,i,&len);
	https->host.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	https->port.data=(char*)lua_tolstring(l,i,&len);
	https->port.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	https->url.data=(char*)lua_tolstring(l,i,&len);
	https->url.len=len;
	if(lua_gettop(l)==4){
		++i;
		if(!lua_isnumber(l,i)){goto end;}
		https->timeout=lua_tonumber(l,i);
	}else{
		https->timeout=-1;
	}
	qtk_https_get(https);
	//wtk_http_response_print(hc->req->response);
	if(https->rbuf->pos>0)
	{
		lua_pushlstring(l,https->rbuf->data,https->rbuf->pos);
		cnt=1;
	}
	qtk_https_delete(https);
end:
	return cnt;
}

int qtk_lua_https_post(lua_State *l)
{
	int i;
	int cnt=0;
	size_t len;
	//int timeout;
	qtk_https_t *https = NULL;
	wtk_string_t *str = NULL;
	https = qtk_https_new();
	str = wtk_string_new(0);

	i=1;
	if(!lua_isstring(l,i)){goto end;}
	https->host.data=(char*)lua_tolstring(l,i,&len);
	https->host.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	https->port.data=(char*)lua_tolstring(l,i,&len);
	https->port.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	https->url.data=(char*)lua_tolstring(l,i,&len);
	https->url.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	str->data = (char*)lua_tolstring(l,i,&len);
	str->len = len;
	if(lua_gettop(l)==5){
		++i;
		if(!lua_isnumber(l,i)){goto end;}
		https->timeout=lua_tonumber(l,i);
	}else{
		https->timeout=-1;
	}

	qtk_https_post(https,str);
	//wtk_http_response_print(hc->req->response);
	if(https->rbuf->pos>0)
	{
		lua_pushlstring(l,https->rbuf->data,https->rbuf->pos);
		cnt=1;
	}
	wtk_string_delete(str);
	qtk_https_delete(https);
end:
	return cnt;
}
#include "wtk/core/wtk_str_encode.h"
#include "wtk/core/wtk_gbk.h"

int wtk_lua_utf8_to_gbk(lua_State *l)
{
	wtk_string_t v;
	size_t len;
	int cnt=0;
	int i;
	char *t;

	i=1;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	//t=wtk_utf8_to_gbk(v.data,v.len);
#ifdef __ANDROID__
	t=wtk_utf8_to_gbk(v.data,v.len);
#elif WIN32
	t = utf8_to_gbk(v.data);
#else 
	t=utf8_to_gbk_2(v.data,v.len);
#endif
	if(!t){goto end;}
	lua_pushstring(l,t);
	cnt=1;
	wtk_free(t);
end:
	return cnt;
}

typedef struct
{
	lua_State *l;
	int i;
}wtk_lua_os_list_t;

void wtk_lua_os_list_add(wtk_lua_os_list_t *lt,char *name,int len)
{
	wtk_string_t *v;
	char *p;

	if(len>0)
	{
		v=wtk_basename(name,'/');
		p=wtk_str_rchr(v->data,v->len,'.');
		name=v->data;
		len=(int)(p-v->data);
		++lt->i;
		lua_pushnumber(lt->l,lt->i);
		lua_pushlstring(lt->l,name,len);
		lua_settable(lt->l,-3);
		wtk_string_delete(v);
	}
}

int wtk_lua_list_dir(lua_State *l)
{
	char *p;
	int cnt=0;
	int i;
	wtk_lua_os_list_t lt;

	i=1;
	if(!lua_isstring(l,i)){goto end;}
	p=(char*)lua_tostring(l,i);
	lt.l=l;
	lt.i=0;
	lua_newtable(l);
	wtk_os_dir_walk(p,&lt,(wtk_os_dir_walk_notify_f)wtk_lua_os_list_add);
	cnt=1;
end:
	return cnt;
}

#ifdef __ANDROID__
#include <android/log.h>
#else
#endif

int wtk_lua_log(lua_State *l)
{
	char *p;
	int i;

	i=1;
	if(!lua_isstring(l,i)){goto end;}
	p=(char*)lua_tostring(l,i);
#ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_DEBUG,"wtk",p);
#else
	printf("%s\n",p);
#endif
end:
	return 0;
}

int wtk_lua_print_HEX(lua_State *l)
{
	wtk_string_t v;
	size_t len;
	int cnt=0;
	int i;
	wtk_strbuf_t *buf;

	i=1;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	buf=wtk_strbuf_new(256,1);
	for(i=0;i<v.len;++i)
	{
		//printf("\\x%02X",(unsigned char)v.data[i]);
		wtk_strbuf_push_f(buf,"%%%02X",(unsigned char)v.data[i]);
	}
	lua_pushlstring(l,buf->data,buf->pos);
	wtk_strbuf_delete(buf);
	cnt=1;
end:
	return cnt;
}

void wtk_lua_sqlite_bind_json(void *ths,int index,int col,sqlite3_value *v)
{
	int type;
	char *p;
	int n;

	//wtk_debug("[%d,%d]\n",index,col);
	type=sqlite3_value_type(v);
	//wtk_debug("type=%d\n",type);
	switch(type)
	{
	case SQLITE_INTEGER:
		wtk_debug("%d\n",sqlite3_value_int(v));
		break;
	case SQLITE_FLOAT:
		wtk_debug("%f\n",sqlite3_value_double(v));
		break;
	case SQLITE_BLOB:
		p=(char*)sqlite3_value_blob(v);
		n=sqlite3_value_bytes(v);
		wtk_debug("[%.*s]\n",n,p);
		break;
	case SQLITE_NULL:
		break;
	case SQLITE_TEXT:
		p=(char*)sqlite3_value_text(v);
		wtk_debug("[%s]=%d\n",p,(int)strlen(p));
		break;
	default:
		break;
	}
}

int wtk_sqlite_exe_json_lua(wtk_sqlite_t *s,wtk_string_t *sql,wtk_json_t *json,int random_one)
{
	sqlite3_stmt* stmt=0;
	sqlite3* db=s->db;
	sqlite3_value *v;
	int ret;
	int i,ncol;
	wtk_json_item_t *item;
	int type;
	char *p;

	json->main=wtk_json_new_array(json);
	//wtk_debug("%.*s\n",sql->len,sql->data);
	ret=sqlite3_prepare(db,sql->data,sql->len,&stmt,0);
	if(ret!=SQLITE_OK)
	{
		//wtk_debug("ret=%d\n",ret);
		wtk_debug("prepare[%.*s],ret=%d failed.\n",sql->len,sql->data,ret);
		ret=-1;goto end;
	}
	while(1)
	{
		ret=sqlite3_step(stmt);
		//wtk_debug("%d\n",ret);
		if(ret!=SQLITE_ROW)
		{
			break;
		}
		item=wtk_json_new_array(json);
		ncol=sqlite3_column_count(stmt);
		for(i=0;i<ncol;++i)
		{
			v=sqlite3_column_value(stmt,i);
			type=sqlite3_value_type(v);
			switch(type)
			{
			case SQLITE_INTEGER:
				wtk_json_array_add_ref_number(json,item,sqlite3_value_int(v));
				break;
			case SQLITE_FLOAT:
				wtk_json_array_add_ref_number(json,item,sqlite3_value_double(v));
				break;
			case SQLITE_TEXT:
				p=(char*)sqlite3_value_text(v);
				wtk_json_array_add_str(json,item,p,strlen(p));
				break;
			default:
				break;
			}
		}
		wtk_json_array_add_item(json,json->main,item);
	}
	ret=ret==SQLITE_DONE?0:-1;
end:
	if(stmt)
	{
		sqlite3_finalize(stmt);
	}
	return ret;
}

int wtk_lua_semdlg_set_next_json_handler(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_semfld_t *fld;
	int i;
	wtk_string_t k;
	size_t len;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isuserdata(l,i))
	{
		wtk_semdlg_set_next_json_handler(sem,NULL,NULL);
		goto end;
	}
	fld=(wtk_semfld_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	wtk_semdlg_set_next_json_handler(sem,fld,&k);
end:
	return 0;
}


int wtk_lua_semdlg_get_data(lua_State *l)
{
	wtk_semdlg_t *sem;
	int i;
	wtk_string_t k;
	size_t len;
	int cnt=0;
	wtk_json_t *json;
	int n;
	int random_one=0;

	n=lua_gettop(l);
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	if(!sem->db){goto end;}
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	if(n==3)
	{
		++i;
		if(lua_type(l,i)!=LUA_TBOOLEAN){goto end;}
		random_one=lua_toboolean(l,i);
	}
	//wtk_debug("[%.*s]\n",k.len,k.data);
	json=wtk_json_new();
	wtk_sqlite_exe_json_lua(sem->db,&k,json,random_one);
	wtk_lua_push_json(l,json->main);
	//wtk_json_item_print3(json->main);
	//exit(0);
	wtk_json_delete(json);
	//lua_pushnumber(l,f);
	cnt=1;
end:
	return cnt;
}


int wtk_lua_semdlg_chnlike(lua_State *l)
{
	wtk_semdlg_t *sem;
	int i;
	wtk_string_t k1,k2;
	size_t len;
	float f;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	if(!sem->chnlike){goto end;}
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k1.data=(char*)lua_tolstring(l,i,&len);
	k1.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k2.data=(char*)lua_tolstring(l,i,&len);
	k2.len=len;
	f=wtk_chnlike_like(sem->chnlike,k1.data,k1.len,k2.data,k2.len,NULL);
	lua_pushnumber(l,f);
	cnt=1;
end:
	return cnt;
}

int wtk_lua_semdlg_feed_robot(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t k,v;
	size_t len;
	int i;
	int n;

	n=lua_gettop(l);
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	if(n==3)
	{
		++i;
		//wtk_debug("n=%d\n",n);
		if(!lua_isstring(l,i)){goto end;}
		v.data=(char*)lua_tolstring(l,i,&len);
		v.len=len;
		wtk_semdlg_feed_robot_msg2(sem,k.data,k.len,v.data,v.len);
	}else
	{
		wtk_semdlg_feed_robot_msg(sem,k.data,k.len);
	}
end:
	return 0;
}

int wtk_lua_semdlg_flush(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t fld,v;
	size_t len;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	fld.data=(char*)lua_tolstring(l,i,&len);
	fld.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	v=wtk_semdlg_flush(sem,&fld,&(v));
	if(v.len>0)
	{
		lua_pushlstring(l,v.data,v.len);
		cnt=1;
	}
end:
	return cnt;
}


int wtk_lua_time_get_ms(lua_State *l)
{
	lua_pushnumber(l,time_get_ms());
	return 1;
}

int wtk_lua_wtk_random(lua_State *l)
{
	int f,t;
	int i;
	int cnt;

	cnt=0;
	i=1;
	if(!lua_isnumber(l,i)){goto end;}
	f=lua_tonumber(l,i);
	++i;
	if(!lua_isnumber(l,i)){goto end;}
	t=lua_tonumber(l,i);
	i=wtk_random(f,t);
	lua_pushnumber(l,i);
	cnt=1;
end:
	return cnt;
}

int wtk_lua_msleep(lua_State *l)
{
	float f;
	int i;

	i=1;
	if(!lua_isnumber(l,i)){goto end;}
	f=lua_tonumber(l,i);
	wtk_msleep(f);
end:
	return 0;
}

#include "wtk/core/wtk_os.h"

int wtk_lua_file_exist(lua_State *l)
{
	char *s;
	int i;
	int b=0;

	i=1;
	if(!lua_isstring(l,i)){goto end;}
	s=(char*)lua_tostring(l,i);
	if(wtk_file_exist(s)==0)
	{
		b=1;
	}
end:
	lua_pushboolean(l,b);
	return 1;
}

int wtk_lua_calendar_to_solar(lua_State *l)
{
	wtk_solar_t solar;
	wtk_lunar_t lunar;
	int i;
	int ret=0;

	i=1;
	if(!lua_isnumber(l,i)){goto end;}
	lunar.year=(int)lua_tonumber(l,i);
	i++;
	if(!lua_isnumber(l,i)){goto end;}
	lunar.month=(int)lua_tonumber(l,i);
	i++;
	if(!lua_isnumber(l,i)){goto end;}
	lunar.day=(int)lua_tonumber(l,i);
	i++;
	if(!lua_isboolean(l,i)){goto end;}
	lunar.is_leap=(int)lua_toboolean(l,i);
	//wtk_debug("calendar solar [lunar.is_leap=%d].\n",lunar.is_leap);
	ret=wtk_calendar_to_solar(&(lunar),&(solar));
	if(ret==-1){ret=0;goto end;}
	lua_pushnumber(l,solar.year);
	lua_pushnumber(l,solar.month);
	lua_pushnumber(l,solar.day);
	ret=3;
end:
	return ret;
}

int wtk_lua_calendar_to_lunar(lua_State *l)
{
	wtk_solar_t solar;
	wtk_lunar_t lunar;
	int i;
	int ret=0;

	i=1;
	if(!lua_isnumber(l,i)){goto end;}
	solar.year=(int)lua_tonumber(l,i);
	i++;
	if(!lua_isnumber(l,i)){goto end;}
	solar.month=(int)lua_tonumber(l,i);
	i++;
	if(!lua_isnumber(l,i)){goto end;}
	solar.day=(int)lua_tonumber(l,i);
	ret=wtk_calendar_to_lunar(&(solar),&(lunar));
	if(ret==-1){ret=0;goto end;}
	lua_pushnumber(l,lunar.year);
	lua_pushnumber(l,lunar.month);
	lua_pushnumber(l,lunar.day);
	lua_pushboolean(l,lunar.is_leap);
	//wtk_debug("is_leap=%d\n",lunar.is_leap);
	ret=4;
end:
	return ret;
}

#include "wtk/os/wtk_socket.h"

int wtk_lua_socket_connect(lua_State *l)
{
	char *s;
	char *p;
	int timeout;
	int i;
	int fd;
	int ret;
	int cnt=0;

	//wtk_debug("================================\n");
	i=1;
	if(!lua_isstring(l,i)){goto end;}
	s=(char*)lua_tostring(l,i);
	++i;
	//wtk_debug("================================\n");
	if(!lua_isstring(l,i)){goto end;}
	p=(char*)lua_tostring(l,i);
	++i;
	//wtk_debug("================================\n");
	if(!lua_isnumber(l,i)){goto end;}
	timeout=lua_tonumber(l,i);
	//wtk_debug("==================== %s:%s ============\n",s,p);
	ret=wtk_socket_connect4(s,p,&fd);
	//wtk_debug("================================\n");
	if(ret!=0)
	{
		//perror(__FUNCTION__);
		//wtk_debug("connect[%s:%s] failed ret=%d\n",s,p,ret);
		goto end;
	}
	wtk_socket_set_timeout(fd,timeout);
	lua_pushnumber(l,fd);
	cnt=1;
end:
	//wtk_debug("connect[%s:%s] failed ret=%d\n",s,p,ret);
	return cnt;
}

int wtk_lua_socket_send_msg(lua_State *l)
{
	char *msg;
	size_t len;
	int fd;
	int i;
	int ret;
	int cnt=0;
	wtk_strbuf_t *buf;

	buf=wtk_strbuf_new(256,1);
	i=1;
	if(!lua_isnumber(l,i)){goto end;}
	fd=lua_tonumber(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	msg=(char*)lua_tolstring(l,i,&(len));
	//wtk_debug("[%.*s]\n",(int)len,msg);
	ret=wtk_socket_send_strmsg(fd,msg,len);
	if(ret!=0){goto end;}
	ret=wtk_socket_read_strmsg(fd,buf);
	if(ret!=0){goto end;}
	lua_pushlstring(l,buf->data,buf->pos);
	cnt=1;
end:
	wtk_strbuf_delete(buf);
	return cnt;
}

int wtk_lua_socket_close(lua_State *l)
{
	int fd;
	int i;

	i=1;
	if(!lua_isnumber(l,i)){goto end;}
	fd=lua_tonumber(l,i);
	wtk_socket_close_fd(fd);
end:
	return 0;
}

int qtk_lua_get_else_field_list(lua_State *l)
{
	wtk_array_t *array = NULL;
	wtk_string_t **strs = NULL;
	int i = 0, ret = 0;

	lua_pushstring(l,"else_field");
	lua_gettable(l,LUA_REGISTRYINDEX);	 //这里使用了一个公共的registry的东西

	array = (wtk_array_t *)lua_touserdata(l,-1);
	if(NULL == array){
		goto end;
	}
	strs=array->slot;
	lua_newtable(l);
	for(i = 0;i < array->nslot;++i)
	{
		lua_pushnumber(l,i+1);
		lua_pushlstring(l,strs[i]->data,strs[i]->len);
		lua_settable(l,-3);
	}
	ret = 1;
end:
	return ret;
}

void wtk_lua_semfld_link_wtk(wtk_lua2_t *lua2)
{
	wtk_lua2_link_function(lua2,wtk_lua_socket_connect,"wtk_socket_connect");
	wtk_lua2_link_function(lua2,wtk_lua_socket_send_msg,"wtk_socket_send_msg");
	wtk_lua2_link_function(lua2,wtk_lua_socket_close,"wtk_socket_close");

	wtk_lua2_link_function(lua2,wtk_lua_time_get_ms,"time_get_ms");
	wtk_lua2_link_function(lua2,wtk_lua_msleep,"wtk_msleep");
	wtk_lua2_link_function(lua2,wtk_lua_file_exist,"wtk_file_exist");
	wtk_lua2_link_function(lua2,wtk_lua_wtk_random,"wtk_random");

	wtk_lua2_link_function(lua2,wtk_lua_gpio_wiringPiSetup,"wiringPiSetup");
	wtk_lua2_link_function(lua2,wtk_lua_gpio_pinmode,"pinMode");
	wtk_lua2_link_function(lua2,wtk_lua_gpio_digitalWrite,"digitalWrite");
	wtk_lua2_link_function(lua2,wtk_lua_gpio_digitalRead,"digitalRead");
	wtk_lua2_link_function(lua2,wtk_lua_gpio_softPwmWrite,"softPwmWrite");
	wtk_lua2_link_function(lua2,wtk_lua_gpio_pullUpDnControl,"pullUpDnControl");
	wtk_lua2_link_function(lua2,wtk_lua_gpio_softPwmCreate,"softPwmCreate");
}

void wtk_lua_semfld_link(wtk_lua2_t *lua2)
{
	wtk_lua_semfld_link_wtk(lua2);
	//wtk_semfld_get_data_fn
	wtk_lua2_link_function(lua2,wtk_lua_semfld_get_data_fn,"wtk_semfld_get_data_fn");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_set_slot,"wtk_semfld_set_slot");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_set_slot2,"wtk_semfld_set_slot2");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_del_slot,"wtk_semfld_del_slot");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_del_slot2,"wtk_semfld_del_slot2");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_set_output,"wtk_semfld_set_output");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_get_output,"wtk_semfld_get_output");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_get_slot,"wtk_semfld_get_slot");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_get_ask_slot,"wtk_semfld_get_ask_slot");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_set_ask_slot,"wtk_semfld_set_ask_slot");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_get_ask_slot_value,"wtk_semfld_get_ask_slot_value");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_set_end,"wtk_semfld_set_end");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_process_nlg,"wtk_semfld_process_nlg");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_process_nlg2,"wtk_semfld_process_nlg2");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_process_nlg3,"wtk_semfld_process_nlg3");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_reset_slot,"wtk_semfld_reset_slot");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_get_ask_deny_func,"wtk_semfld_get_ask_deny_func");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_set_ask_deny_func,"wtk_semfld_set_ask_deny_func");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_set_fst_state,"wtk_semfld_set_fst_state");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_get_fst_state,"wtk_semfld_get_fst_state");

	//wtk_lua2_link_function(lua2,wtk_lua_semfld_process_lex,"wtk_semfld_process_lex");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_get_dn,"wtk_semfld_get_dn");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_remove_from_ctx,"wtk_semfld_remove_from_ctx");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_process_owl,"wtk_semfld_process_owl");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_get_owl,"wtk_semfld_get_owl");
	wtk_lua2_link_function(lua2,wtk_lua_semfld_goto_state,"wtk_semfld_goto_state");
	//wtk_lua2_link_function(lua2,wtk_lua_semfld_get_owlfst,"wtk_semfld_get_owlfst");


	wtk_lua2_link_function(lua2,wtk_lua_semdlg_get_last_fld,"wtk_semdlg_get_last_fld");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_get_fld,"wtk_semdlg_get_fld");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_set_next_fld,"wtk_semdlg_set_next_fld");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_set_last_fld,"wtk_semdlg_set_last_fld");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_get_input,"wtk_semdlg_get_input");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_process,"wtk_semdlg_process");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_has_process_domain,"wtk_semdlg_has_process_domain");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_set_ext_json,"wtk_semdlg_set_ext_json");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_get_ext_input,"wtk_semdlg_get_ext_input");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_get_last_input,"wtk_semdlg_get_last_input");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_get_last_output,"wtk_semdlg_get_last_output");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_add_faq,"wtk_semdlg_add_faq");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_get_faq,"wtk_semdlg_get_faq");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_get_faq2,"wtk_semdlg_get_faq2");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_set_output,"wtk_semdlg_set_output");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_last_is_faq,"wtk_semdlg_last_is_faq");

	wtk_lua2_link_function(lua2,wtk_lua_semdlg_syn,"wtk_semdlg_syn");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_play_file,"wtk_semdlg_play_file");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_set_rec_grammar,"wtk_semdlg_set_rec_grammar");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_exe,"wtk_semdlg_exe");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_get_conf,"wtk_semdlg_get_conf");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_get_vadtime,"wtk_semdlg_get_vad_time");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_kv_get_dat,"wtk_semdlg_kv_get_dat");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_kv_set_dat,"wtk_semdlg_kv_set_dat");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_kv_get,"wtk_semdlg_kv_get");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_kv_set,"wtk_semdlg_kv_set");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_datkv_get,"wtk_semdlg_datkv_get");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_datkv_get_random,"wtk_semdlg_datkv_get_random");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_get_like,"wtk_semdlg_get_like");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_get_wrdvec_like,"wtk_semdlg_get_wrdvec_like");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_get_env,"wtk_semdlg_get_env");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_quit_fld,"wtk_semdlg_quit_fld");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_remove_history,"wtk_semdlg_remove_history");

	wtk_lua2_link_function(lua2,wtk_lua_semdlg_set_has_post_audio,"wtk_semdlg_set_has_post_audio");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_feed_robot,"wtk_semdlg_feed_robot");

	wtk_lua2_link_function(lua2,wtk_lua_semdlg_flush,"wtk_semdlg_flush");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_chnlike,"wtk_semdlg_chnlike");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_get_data,"wtk_semdlg_get_data");
	wtk_lua2_link_function(lua2,wtk_lua_semdlg_set_next_json_handler,"wtk_semdlg_set_next_json_handler");

	wtk_lua2_link_function(lua2,wtk_lua_calendar_to_solar,"wtk_calendar_to_solar");
	wtk_lua2_link_function(lua2,wtk_lua_calendar_to_lunar,"wtk_calendar_to_lunar");

	wtk_lua2_link_function(lua2,wtk_lua_http_get,"wtk_http_get");
	wtk_lua2_link_function(lua2,qtk_lua_https_get,"qtk_https_get");
	wtk_lua2_link_function(lua2,qtk_lua_https_post,"qtk_https_post");
	wtk_lua2_link_function(lua2,wtk_lua_utf8_to_gbk,"wtk_utf8_to_gbk");
	wtk_lua2_link_function(lua2,wtk_lua_print_HEX,"wtk_print_HEX");
	wtk_lua2_link_function(lua2,wtk_lua_list_dir,"wtk_list_dir");
	wtk_lua2_link_function(lua2,wtk_lua_log,"wtk_log");
	wtk_lua2_link_function(lua2,qtk_lua_get_else_field_list,"qtk_get_else_field_list"); //得到不是我们系统的一些领域的列表
}


