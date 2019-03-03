#include "wtk/core/wavehdr.h"
#include "wtk_os.h"
#include <ctype.h>
#ifdef WIN32
#include <windows.h>
#include <time.h>
#include <sys/timeb.h>
#include <locale.h>
#else
#include <sys/time.h>
#define __USE_GNU
#include <sys/resource.h>
#endif
#ifdef WIN32
#define localtime_r localtime
#endif


#ifdef WIN32
#define   _WIN32_WINNT   0x0500
#endif
//------------------------ directory and file section ---------------------------

int wtk_mkdir(char* dn)
{
#ifdef WIN32
	return (wtk_file_exist(dn)==0) ? 0 : mkdir(dn);
#else
	return (wtk_file_exist(dn)==0) ? 0 : mkdir(dn,0777);
#endif
}

int wtk_mkdir_p(char* fn,char sep,int create_last_entry)
{
	char *p,*t;
	int ret;

	ret=-1;
	p=wtk_str_dup(fn);
	if(!p){goto end;}
	for(t=p;;++t)
	{
		if( (*t==sep) || (*t==0) )
		{
			if(*t==0 && !create_last_entry)
			{
				break;
			}else if(t==p)
			{
				continue;
			}
			*t=0;
			ret=wtk_mkdir(p);
			if(ret!=0)
			{
				perror(__FUNCTION__);
				printf("%d:[%s] create failed.\n",ret,p);
				goto end;
			}
			if( *(fn+(t-p)) ==  DIR_SEP)
			{
				*t=DIR_SEP;
			}else
			{
				break;
			}
		}
	}
	ret=0;
end:
	if(p){free(p);}
	return ret;
}

void wtk_real_fn(char *fn,int fn_bytes,wtk_strbuf_t *buf,char sep)
{
	wtk_string_t dir;
	char *s,*e;
	int last_pos=0;
	int found_sep=0;

	//wtk_debug("%.*s\n",fn_bytes,fn);
	s=fn;e=s+fn_bytes;
	wtk_string_set(&(dir),s,0);
	wtk_strbuf_reset(buf);
	while(s<e)
	{
		if(*s==sep)
		{
			if(dir.len==0)
			{
				if(buf->pos==0)
				{
					//wtk_strbuf_push_c(buf,*s);
				}
			}else
			{
				if(dir.len==1 && dir.data[0]=='.')
				{
					//process current directory;
					//if(buf->pos==0)
					if(buf->pos==0)
					{
						if(buf->pos>0)
						{
							wtk_strbuf_push_c(buf,*s);
						}
						wtk_strbuf_push(buf,dir.data,dir.len);
					}
				}else if(dir.len==2 && dir.data[0]=='.' && dir.data[1]=='.')
				{
					//wtk_debug("[%.*s]\n",buf->pos,buf->data);
					//if(buf->pos==0)
					if(!found_sep)
					{
						if(buf->pos>0)
						{
							wtk_strbuf_push_c(buf,*s);
						}
						wtk_strbuf_push(buf,dir.data,dir.len);
					}else
					{
						buf->pos=last_pos;
						last_pos=buf->pos;
					}
				}else
				{
					last_pos=buf->pos;
 					if (buf->pos || (buf->pos == 0 && fn[0] == sep)) {
						wtk_strbuf_push_c(buf,*s);
					}
					wtk_strbuf_push(buf,dir.data,dir.len);
					found_sep=1;
				}
				//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			}
			//wtk_debug("[%.*s]\n",dir.len,dir.data);
			wtk_string_set(&(dir),s+1,0);
		}else
		{
			++dir.len;
		}
		++s;
	}
	if(dir.len>0)
	{
		wtk_strbuf_push_c(buf,sep);
		wtk_strbuf_push(buf,dir.data,dir.len);
	}
}

wtk_string_t wtk_dir_name2(char *data,int len,char sep)
{
	char *s,*e;
	wtk_string_t v;

	s=data;e=s+len-1;
	while(e>s)
	{
		if(*e==sep)
		{
			--len;
			break;
		}
		--e;--len;
	}
	wtk_string_set(&v,data,len);
	return v;
}


wtk_string_t* wtk_dir_name(char *fn,char sep)
{
	int len;
	char *s,*e;
	wtk_string_t *str;

	len=strlen(fn);
	s=fn;e=s+len;
	while(e>s)
	{
		if(*e==sep){break;}
		--e;--len;
	}
	//printf("%d:%*.*s#\n",len,len,len,fn);
	str=wtk_string_dup_data(fn,len);
	return str;
}


#ifdef WIN32
double time_get_cpu()
{
    LARGE_INTEGER lt,ft;
    double v;

    QueryPerformanceFrequency(&ft);
    QueryPerformanceCounter(&lt);
    v=lt.QuadPart*1000.0/ft.QuadPart;
    return v;
}

double time_get_ms()
{
    struct timeb tm;
    double ret;

    ftime(&tm);
    ret=tm.time*1000;
    ret+=tm.millitm;
    return ret;
}
#else

double time_get_cpu()
{
#if defined __UNIX__ 
	struct rusage u;
	double v;
	int ret;

	ret=getrusage(RUSAGE_THREAD,&u);
	if(ret==0)
	{
		v=u.ru_utime.tv_sec*1000.0+u.ru_utime.tv_usec*1.0/1000
			+u.ru_stime.tv_sec*1000.0+u.ru_stime.tv_usec*1.0/1000;
	}else
	{
		v=0;
	}
	return v;
#else
	return time_get_ms();
#endif
}

double time_get_ms()
{
    struct timeval tv;
    double ret;
    int err;

    err=gettimeofday(&tv,0);
    if(err==0)
    {
        ret=tv.tv_sec*1000.0+tv.tv_usec*1.0/1000;
        //ret maybe is NAN
        if(ret!=ret)
        {
            wtk_debug("NAN(%.0f,sec=%.d,usec=%.d).\n",ret,(int)tv.tv_sec,(int)tv.tv_usec);
            ret=0;
        }
    }else
    {
        perror(__FUNCTION__);
        ret=0;
    }
    return ret;
}

#if defined __IPHONE_OS__
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

int wtk_dir_walk2(char *dir,void *ths,wtk_dir_walk2_f walk)
{
	DIR *d;
	struct dirent *ent;
	int ret;
	wtk_strbuf_t *buf;
	struct stat xd;

	//wtk_debug("import dir:%s\n",dir);
	buf=wtk_strbuf_new(1024,1);
	d=opendir(dir);
	if(!d){ret=-1;goto end;}
	ret=0;
	while(1)
	{
		ent=readdir(d);
		if(!ent){break;}
		//wtk_debug("[%s]\n",ent->d_name);
		if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
		{
			ret=0;
		}else
		{
			wtk_strbuf_reset(buf);
			wtk_strbuf_push_string(buf,dir);
			wtk_strbuf_push_c(buf,'/');
			wtk_strbuf_push_string(buf,ent->d_name);
			wtk_strbuf_push_c(buf,0);
			ret=stat(buf->data,&xd);
			if(ret==0)
			{
				if(S_ISDIR(xd.st_mode))
				{
					ret=wtk_dir_walk2(buf->data,ths,walk);
				}else
				{
					ret=walk(ths,buf->data);
				}
			}else
			{
				ret=0;
			}
		}
		//wtk_debug("ret=%d\n",ret);
		if(ret!=0){goto end;}
	}
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

int wtk_dir_walk(char *dir,void *ths,wtk_dir_walk_f walk)
{
	DIR *d;
	struct dirent *ent;
	int ret;
	wtk_strbuf_t *buf;
	wtk_string_t v;

	//wtk_debug("import dir:%s\n",dir);
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
			wtk_strbuf_push_string(buf,dir);
			wtk_strbuf_push_c(buf,'/');
			wtk_strbuf_push_string(buf,ent->d_name);
			v.data=buf->data;
			v.len=buf->pos;
			wtk_strbuf_push_c(buf,0);
			ret=walk(ths,&(v));
		}else if(ent->d_type==DT_DIR)
		{
			if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
			{
				ret=0;
			}else
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push_string(buf,dir);
				wtk_strbuf_push_c(buf,'/');
				wtk_strbuf_push_string(buf,ent->d_name);
				wtk_strbuf_push_c(buf,0);
				//wtk_debug("%s[%s:%s]\n",buf->data,ent->d_name,dir);
				ret=wtk_dir_walk(buf->data,ths,walk);
			}
		}else
		{
			ret=0;
		}
		//wtk_debug("ret=%d\n",ret);
		if(ret!=0){goto end;}
	}
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
#endif
#endif

uint64_t file_length(FILE *f)
{
	uint64_t len;

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);
    return len;
}

uint64_t wtk_file_size(char *fn)
{
	uint64_t fs=0;
	FILE *f;

	f=fopen(fn,"rb");
	if(f)
	{
		fs=file_length(f);
		fclose(f);
	}
	return fs;
}

char* file_read_buf(char* fn, int *n)
{
	FILE* file = fopen(fn, "rb");
	char* p = 0;
	int len;

	if (file)
	{
        len=file_length(file);
		p = (char*) wtk_malloc(len + 1);
		len = fread(p, 1, len, file);
		if (n)
		{
			*n = len;
		}
		fclose(file);
		p[len] = 0;
	}
	return p;
}

char* file_read_buf2(char* fn, int fn_len,int *n)
{
	char *v;
	char *data;

	v=(char*)wtk_malloc(fn_len+1);
	memcpy(v,fn,fn_len);
	v[fn_len]=0;
	data=file_read_buf(v,n);
	wtk_free(v);
	return data;
}

#define FILE_STATE_NOERR 0
#define FILE_STATE_END -1
#define FILE_STATE_ERR -2

int file_state(FILE* file)
{
	int state;

	if (feof(file))
	{
		state = FILE_STATE_END;
	}
	else if (ferror(file))
	{
		state = FILE_STATE_ERR;
	}
	else
	{
		state = FILE_STATE_NOERR;
	}
	return state;
}

int file_write(FILE* file, const char* data, int len, int* writed)
{
	int ret = 0, index = 0, nLeft = len;

	if (!data)
	{
		return -1;
	}
	while (nLeft > 0)
	{
		ret = fwrite(&data[index], 1, nLeft, file);
		if (ret < nLeft)
		{
			ret = file_state(file);
			if (ret != FILE_STATE_NOERR)
			{
				break;
			}
		}
		index += ret;
		nLeft -= ret;
	}
	if (writed)
	{
		*writed = index;
	}
	return ret;
}

int file_write_buf(char* fn, const char* data, size_t len)
{
	FILE* file;
	int ret = -1;

	wtk_mkdir_p(fn,DIR_SEP,0);
	file = fopen(fn, "wb");
	if (file)
	{
		ret = file_write(file, data, len, NULL);
		fclose(file);
		ret=ret==len?0:-1;
	}
	return ret;
}

#ifdef WIN32
int wtk_dir_walk(const char* path,wtk_dir_walk_handler_t cb,void* user_data)
{
    char buf[2048];
    WIN32_FIND_DATA data;
    HANDLE hFind;
    int ret;
    char* p=buf;
    int p_size=sizeof(buf);
    size_t count;

    count=strlen(path)+3;
    if(count>p_size)
    {
        p_size=count<<1;
        p=(char*)wtk_malloc(sizeof(char)*p_size);
    }
    count=_snprintf(p,p_size,"%s\\*",path);
    hFind=FindFirstFile(p,&data);
    if(hFind!=INVALID_HANDLE_VALUE)
    {
        ret=0;
        do
        {
            count=strlen(data.cFileName);
            count=_snprintf(p,p_size,"%s\\%s",path,data.cFileName);
            if(count>=p_size||count<0)
            {
                if(p!=buf)
                {
                    free(p);
                }
                p_size=strlen(path)+strlen(data.cFileName)+3;
                p=(char*)wtk_malloc(sizeof(char)*p_size);
                count=_snprintf(p,p_size,"%s\\%s",path,data.cFileName);
            }
            //printf("%s\n",p);
            if(data.dwFileAttributes==FILE_ATTRIBUTE_DIRECTORY)
            {
                if(strcmp(data.cFileName,".")!=0&&strcmp(data.cFileName,"..")!=0)
                {
                    ret=wtk_dir_walk(p,cb,user_data);
                    if(ret!=0)
                    {
                        break;
                    }
                }
            }else if(cb)
            {
                if(cb(user_data,p)!=0)
                {
                    ret=-1;
                    break;
                }
            }
        }while(FindNextFile(hFind,&data));
        FindClose(hFind);
    }else
    {
        ret=-1;
    }
    if(p!=buf)
    {
        free(p);
    }
    return ret;
}
#endif

#ifdef WIN32

char* wcs_to_mbs(const wchar_t* wcs,int *ret_count)
{
    char* mbs=0;
    int c;

    if(wcs)
    {
        c=0;
        c=wcstombs(NULL,wcs,c);
        if(c>0)
        {
            c+=1;
            mbs=(char*)wtk_calloc(1,c);
            c=wcstombs(mbs,wcs,c-1);
            if(c==-1)
            {
                free(mbs);
                mbs=0;
            }
            if(ret_count)
            {
                *ret_count=c;
            }
        }
    }
    if(!mbs && ret_count)
    {
        *ret_count=0;
    }
    return mbs;
}

/*
void dir_monitor(const char* path,  dir_monitor_handler cb,void* user_data,HANDLE* handle)
{
    FILE_NOTIFY_INFORMATION *tmp;
    char fullpath[2048];
    char buf[2048];
    DWORD readed;
    int ret, len_path,len_fn;
    HANDLE hdr;
    char* fn;
    char* p;

    if(!wtk_exist(path)) {mkdir(path);}
    ret=GetFullPathNameA(path,sizeof(fullpath),fullpath,0);
    if(ret==0){goto end;}
    len_path=strlen(fullpath);
    hdr=CreateFileA(fullpath,FILE_LIST_DIRECTORY,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_ALWAYS,FILE_FLAG_BACKUP_SEMANTICS,NULL);
    if(handle){*handle=hdr;}
    while(ReadDirectoryChangesW(hdr,buf,sizeof(buf),TRUE,FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_LAST_WRITE,&readed,NULL,NULL))
    {
        if(readed>0)
        {
            while(1)
            {
                tmp=(FILE_NOTIFY_INFORMATION*)buf;
                if(tmp->Action==FILE_ACTION_MODIFIED && tmp->FileNameLength>0&&cb)
                {
                    tmp->FileName[tmp->FileNameLength/2]=0;
                    tmp->FileName[tmp->FileNameLength/2+1]=0;
                    fn=wcs_to_mbs(tmp->FileName,&len_fn);
                    if(fn)
                    {
                        p=wtk_malloc(len_path+len_fn+2);
                        sprintf(p,"%s\\%s",fullpath,fn);
                        if(cb(user_data,p)!=0)
                        {
                            goto end;
                        }
                        free(fn); free(p);
                    }
                }
                if(tmp->NextEntryOffset==0)
                {
                    break;
                }
                tmp=(FILE_NOTIFY_INFORMATION*)((char*)tmp+tmp->NextEntryOffset);
            }
        }
    }
end:
    CloseHandle(hdr);
    return;
}

int dir_walk(const char* path,dir_monitor_handler cb,void* user_data)
{
    char buf[2048];
    WIN32_FIND_DATA data;
    HANDLE hFind;
    int ret;
    char* p=buf;
    int p_size=sizeof(buf);
    size_t count;

    count=strlen(path)+3;
    if(count>p_size)
    {
        p_size=count<<1;
        p=(char*)wtk_malloc(sizeof(char)*p_size);
    }
    count=_snprintf(p,p_size,"%s\\*",path);
    hFind=FindFirstFile(p,&data);
    if(hFind!=INVALID_HANDLE_VALUE)
    {
        ret=0;
        do
        {
            count=strlen(data.cFileName);
            count=_snprintf(p,p_size,"%s\\%s",path,data.cFileName);
            if(count>=p_size||count<0)
            {
                if(p!=buf)
                {
                    free(p);
                }
                p_size=strlen(path)+strlen(data.cFileName)+3;
                p=(char*)wtk_malloc(sizeof(char)*p_size);
                count=_snprintf(p,p_size,"%s\\%s",path,data.cFileName);
            }
            //printf("%s\n",p);
            if(data.dwFileAttributes==FILE_ATTRIBUTE_DIRECTORY)
            {
                if(strcmp(data.cFileName,".")!=0&&strcmp(data.cFileName,"..")!=0)
                {
                    ret=dir_walk(p,cb,user_data);
                    if(ret!=0)
                    {
                        break;
                    }
                }
            }else if(cb)
            {
                if(cb(user_data,p)!=0)
                {
                    ret=-1;
                    break;
                }
            }
        }while(FindNextFile(hFind,&data));
        FindClose(hFind);
    }else
    {
        ret=-1;
    }
    if(p!=buf)
    {
        free(p);
    }
    return ret;
}
*/
#endif


char* wtk_realpath(char *fn,char *buf)
{
#ifdef WIN32
    //GetFullPathNameA(fn,4096,buf,0);
    return 0;
#else
	return realpath(fn,buf);
#endif
}

wtk_string_t* wtk_dirname(char *fn,char sep)
{
	wtk_string_t *p=0;

	if(!fn){goto end;}
	p=wtk_str_left(fn,strlen(fn),sep);
end:
	return p;
}

wtk_string_t* wtk_str_left(char *fn,int len,char sep)
{
	wtk_string_t *p=0;
	char *s,*e;

	if(!fn){goto end;}
	s=fn;e=fn+len-1;
	while(e>=s)
	{
		if(*e==sep)
		{
			p=wtk_string_dup_data(s,e-s);
			break;
		}
		--e;
	}
end:
	return p;
}

wtk_string_t* wtk_basename(char* fn,char sep)
{
	wtk_string_t *p=0;

	if(!fn){goto end;}
	p=wtk_str_right(fn,strlen(fn),sep);
end:
	return p;
}

wtk_string_t* wtk_str_right(char* fn,int len,char sep)
{
	wtk_string_t *p=0;
	char *s,*e,*t;

	if(!fn){goto end;}
	s=fn;e=fn+len-1;
	t=e;
	while(t>s)
	{
		if(*t==sep)
		{
			++t;
			break;
		}
		--t;
	}
	p=wtk_string_dup_data(t,e-t+1);
end:
	return p;
}

wtk_string_t* wtk_real_dirname(char *fn)
{
	wtk_string_t *str=0;
	char buf[4096];
	char *p;

	p=wtk_realpath(fn,buf);
	if(!p){goto end;}
	str=wtk_dirname(p,DIR_SEP);
end:
	return str;
}

int wtk_gcc_month()
{
	static char* month[]={
			"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
	};
	int n=sizeof(month)/sizeof(char*);
	int i,m;

	for(i=0,m=0;i<n;++i)
	{
		if(strncmp(month[i],__DATE__,3)==0)
		{
			m=i+1;
		}
	}
	return m;
}

int wtk_gcc_day()
{
	char *p=__DATE__;
	char *s,*t;
	int v;

	s=p+4;
	while(isdigit(*s)==0)
	{
		++s;
	}
	t=s;
	while(isdigit(*s))
	{
		++s;
	}
	v=wtk_str_atoi(t,s-t);
	return v;
}

int wtk_gcc_year()
{
	char *p=__DATE__;
	char *s,*t;
	int v,len;

	len=strlen(p);
	t=p+len;
	while(isdigit(*t)==0)
	{
		--t;
	}
	s=t;
	while(isdigit(*s))
	{
		--s;
	}
	++s;
	v=wtk_str_atoi(s,t-s+1);
	return v;
}

int wtk_get_build_timestamp(char *buf)
{
	int y,m,d;
	int n;

    y=wtk_gcc_year();
    m=wtk_gcc_month();
    d=wtk_gcc_day();
    n=sprintf(buf,"%d.%d.%d.%s",y,m,d,__TIME__);
    return n;
}

int wtk_os_timestamp(char *buf)
{
	time_t ct;
	struct tm xm;
	struct tm* m;
	int ret,n;

	n=0;
	ret=time(&ct);
	if(ret==-1){goto end;}
#ifdef __WIN32__
    m=localtime(&ct);
#else
	m=localtime_r(&ct,&xm);
#endif
	//m=localtime(&ct);
	if(!m){ret=-1;goto end;}
	n=sprintf(buf,"%04d-%02d-%02d-%02d:%02d:%02d.%.0f",m->tm_year+1900,m->tm_mon+1,m->tm_mday,
			m->tm_hour,m->tm_min,m->tm_sec,time_get_ms());
	ret=0;
end:
	return n;
}

int wtk_os_timestamp2(wtk_strbuf_t *buf)
{
	time_t ct;
	struct tm* m;
	int ret,n;

	n=0;
	ret=time(&ct);
	if(ret==-1){goto end;}
	m=localtime(&ct);
	if(!m){ret=-1;goto end;}
	wtk_strbuf_push_f(buf,"%04d%02d%02d-%02d%02d%02d.%.3f",m->tm_year+1900,m->tm_mon+1,m->tm_mday,
			m->tm_hour,m->tm_min,m->tm_sec,time_get_ms());
	ret=0;
end:
	return n;
}

FILE* wtk_file_open(char* fn,char * mode)
{
	FILE* f;
	int ret;

	f=0;
#ifdef WIN32
    ret=wtk_mkdir_p(fn,'\\',0);
#else
	ret=wtk_mkdir_p(fn,'/',0);
#endif
	if(ret!=0){goto end;}
	f=fopen(fn,mode);
end:
	return f;
}

void wtk_msleep(int ms)
{
#ifdef WIN32
    Sleep(ms);
#else
    usleep(ms*1000);
#endif
}

char* wtk_search_file(char *fn,wtk_string_t **path,int n,wtk_strbuf_t *buf)
{
	int i;
	int ret;
	int len;

	ret=wtk_file_exist(fn);
	if(ret==0){return fn;}
	len=strlen(fn);
	for(i=0;i<n;++i)
	{
		//wtk_debug("[%.*s]\n",strs[i]->len,strs[i]->data);
		wtk_strbuf_reset(buf);
		wtk_strbuf_push(buf,path[i]->data,path[i]->len);
		wtk_strbuf_push_c(buf,DIR_SEP);
		wtk_strbuf_push(buf,fn,len);
		wtk_strbuf_push_c(buf,0);
		ret=wtk_file_exist(buf->data);
		//wtk_debug("[%s,ret=%d]\n",buf->data,ret);
		if(ret==0)
		{
			return buf->data;
		}
	}
	return NULL;
}

#ifdef WIN32
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int wtk_is_dir(char *dn)
{
	struct stat buf;
	int ret;

	ret=stat(dn,&buf);
	if(ret!=0){ret=0;goto end;}
	if(S_ISDIR(buf.st_mode))
	{
		ret=1;
	}else
	{
		ret=0;
	}
end:
	return ret;
}
#endif

int wtk_file_lines2(char *fn)
{
	char *data;
	int len;
	int n=0;
	char *s,*e;
	int newline;

	data=file_read_buf(fn,&len);
	if(!data){goto end;}
	s=data;e=s+len;
	newline=1;
	while(s<e)
	{
		if(*s=='\n')
		{
			newline=1;
		}else
		{
			if(newline)
			{
				++n;
				newline=0;
			}
		}
		++s;
	}
end:
	if(data)
	{
		wtk_free(data);
	}
	return n;
}

int wtk_file_read_line2(FILE *f,wtk_strbuf_t *buf,unsigned long long index)
{
#define STEP_SIZE 1024
	char tmp[STEP_SIZE];

	return wtk_file_read_line(f,buf,index,tmp,STEP_SIZE);
}


int wtk_file_read_line(FILE *f,wtk_strbuf_t *buf,unsigned long long index,char *tmp,int tmp_size)
{
typedef enum
{
	WTK_FILE_LINE_INIT,
	WTK_FILE_LINE_FOUND,
}wtk_file_line_state_t;
	int ret;
	unsigned long long i=0;
	char *s,*e;
	wtk_file_line_state_t state;

	wtk_strbuf_reset(buf);
	rewind(f);
	if(i==index)
	{
		state=WTK_FILE_LINE_FOUND;
	}else
	{
		state=WTK_FILE_LINE_INIT;
	}
	s=tmp;
	while(1)
	{
		ret=fread(tmp,1,tmp_size,f);
		if(ret>0)
		{
			s=tmp;
			e=s+ret;
			while(s<e)
			{
				switch(state)
				{
				case WTK_FILE_LINE_INIT:
					if(*s=='\n')
					{
						++i;
						if(i==index)
						{
							state=WTK_FILE_LINE_FOUND;
						}
					}
					break;
				case WTK_FILE_LINE_FOUND:
					if(*s=='\n')
					{
						return 0;
					}else
					{
						wtk_strbuf_push(buf,s,1);
					}
					break;
				}
				++s;
			}
		}
		if(ret!=tmp_size)
		{
			break;
		}
	}
	wtk_strbuf_reset(buf);
	return 0;
}

unsigned long long wtk_file_lines(char *fn)
{
	FILE *f;
	char buf[4096];
	int len;
	unsigned long long n=0;
	char *s,*e;
	//int newline;

	f=fopen(fn,"r");
	if(!f){goto end;}
	//newline=1;
	while(1)
	{
		len=fread(buf,1,sizeof(buf),f);
		if(len<=0){goto end;}
		s=buf;e=s+len;
		while(s<e)
		{
			if(*s=='\n')
			{
				++n;
			}
//			if(*s=='\n')
//			{
//				newline=1;
//			}else
//			{
//				if(newline)
//				{
//					++n;
//					newline=0;
//				}
//			}
			++s;
		}
	}
end:
	if(f)
	{
		fclose(f);
	}
	return n;
}

int wtk_file_copy(char *src,char *dst,char sep)
{
#define FILE_BUF_SIZE 1024
	char buf[FILE_BUF_SIZE];
	FILE *fin=NULL;
	FILE *fout=NULL;
	int ret=-1;
	int len;

	fin=fopen(src,"rb");
	if(!fin){goto end;}
	ret=wtk_mkdir_p(dst,sep,0);
	if(ret!=0){goto end;}
	fout=fopen(dst,"wb");
	if(!fout){goto end;}
	while(1)
	{
		ret=fread(buf,1,FILE_BUF_SIZE,fin);
		if(ret<0){goto end;}
		if(ret==0){break;}
		len=ret;
		ret=fwrite(buf,1,len,fout);
		if(len!=ret){ret=-1;goto end;}
	}
	ret=0;
end:
	if(fout)
	{
		fclose(fout);
	}
	if(fin)
	{
		fclose(fin);
	}
	return ret;
}

int wtk_file_copy2(FILE *fin,char *dst,char sep,int want_len)
{
#define FILE_BUF_SIZE 1024
	char buf[FILE_BUF_SIZE];
	FILE *fout=NULL;
	int ret=-1;
	int len;
	int cnt=0;
	int step;

	ret=wtk_mkdir_p(dst,sep,0);
	if(ret!=0){goto end;}
	fout=fopen(dst,"wb");
	if(!fout){goto end;}
	while(cnt<want_len)
	{
		step=min(want_len-cnt,FILE_BUF_SIZE);
		ret=fread(buf,1,step,fin);
		if(ret<0){goto end;}
		if(ret==0){break;}
		len=ret;
		cnt+=len;
		ret=fwrite(buf,1,len,fout);
		if(len!=ret){ret=-1;goto end;}
	}
	ret=0;
end:
	if(fout)
	{
		fclose(fout);
	}
	return ret;
}

#include <math.h>

int wtk_random(int left,int right)
{
	int i,t;

	if(left==right){return left;}
	//srandom(time_get_ms());
	i=random();
	t=right-left+1;
	//wtk_debug("i=%d t=%d\n",i,t);
	if(t<0)
	{
		t=-t;
	}else if(t==0)
	{
		t=1;
	}
	//wtk_debug("t=%d\n",t);
	i=(i%t) + left;
	//wtk_debug("i=%d\n",i);
	return i;
}

int wtk_random2(int left,int right)
{
	int i,t;
	double xt;
	//int rx;

	if(left==right){return left;}
	//rx=*(&(xt)+2);//make compile happy
	xt=time_get_ms()*10;//+rx;
//	if(xt==0)
//	{
//		xt=clock();
//	}
	//srandom(xt);
	//i=random();
	//
	//t=(int)(xt/100.0);
	//i=xt%1000;//-(int)(xt/10000)*10000);
	i=(int)fmod(xt,1000000);
	//wtk_debug("random=%f/%d\n",xt,i);
	//wtk_debug("=============xxxxx=============> set xt=%f  i=%d\n",xt,i);
	//wtk_debug("i=%d xt=%f\n",i,xt);
	t=right-left+1;
	if(t<0)
	{
		t=-t;
	}else if(t==0)
	{
		t=1;
	}
	//wtk_debug("t=%d\n",t);
	i=(i%t) + left;
	return i;
}

int wtk_os_dir_walk(char *dir,void *ths,wtk_os_dir_walk_notify_f notify)
{
	DIR *d;
	struct dirent *ent;
	int ret;
	wtk_strbuf_t *buf;
	wtk_string_t dx;

	dx.data=dir;
	dx.len=strlen(dir);
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
			wtk_strbuf_push(buf,dx.data,dx.len);
			wtk_strbuf_push_c(buf,'/');
			wtk_strbuf_push_string(buf,ent->d_name);
			wtk_strbuf_push_c(buf,0);
			--buf->pos;
			notify(ths,buf->data,buf->pos);
			//wtk_debug("%s\n",buf->data);
			//v=wtk_basename(buf->data,'/');
			//wtk_debug("[%.*s]\n",v->len,v->data);
			//p=wtk_str_rchr(v->data,v->len,'.');
			//wtk_debug("[%.*s]\n",(int)(p-v->data),v->data);
			//notify(ths,v->data,(int)(p-v->data));
			//wtk_string_delete(v);
			ret=0;
		}else if(ent->d_type==DT_DIR)
		{
			if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
			{
				ret=0;
			}else
			{
				wtk_strbuf_reset(buf);
				//wtk_strbuf_push_string(buf,dir);
				wtk_strbuf_push(buf,dx.data,dx.len);
				wtk_strbuf_push_c(buf,'/');
				wtk_strbuf_push_string(buf,ent->d_name);
				wtk_strbuf_push_c(buf,0);
				wtk_os_dir_walk(dir,ths,notify);
			}
		}else
		{
			ret=0;
		}
		//wtk_debug("ret=%d\n",ret);
		if(ret!=0){goto end;}
	}
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

