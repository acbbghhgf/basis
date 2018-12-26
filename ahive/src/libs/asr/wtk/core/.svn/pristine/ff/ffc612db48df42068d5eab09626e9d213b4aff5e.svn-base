#include "wtk_str_encode.h"
#include <ctype.h>
#ifdef WIN32
#include <winsock2.h>
#include <Windows.h>
#else
#ifdef __ANDROID__
#else
#ifdef __unix__
#define USE_ICONV
#endif
#endif
#endif

#ifdef USE_ICONV
#include <iconv.h>
#endif

#ifdef WIN32
char* str_convert(const char * src,unsigned int  code_from,unsigned int code_to)
{
    wchar_t *u;
    char *dst;
    int ret,len;

    dst=0;u=0;
    ret=-1;
    if(!src){goto end;}
    len = MultiByteToWideChar(code_from, 0,src, -1, NULL,0 );
    if(len<1){goto end;}
    u=(wchar_t*)wtk_malloc( sizeof(wchar_t) * len);
    len = MultiByteToWideChar( code_from, 0, src, -1, u, len );
    if(len<1){goto end;}
    len = WideCharToMultiByte( code_to, 0, u, -1, NULL, 0, NULL, NULL );
    if(len<1){goto end;}
    dst = (char *) wtk_calloc(1, sizeof(char) * len+1);
    len = WideCharToMultiByte( code_to, 0, u, -1, dst, len, NULL, NULL );
    if(len<1){goto end;}
    ret=0;
end:
    if(u){free(u);}
    if(ret!=0 && dst)
    {
        free(dst);
        dst=0;
    }
    return dst;
}

char* utf8_to_gbk( const char * utf_8)
{
    return str_convert(utf_8,CP_UTF8,CP_ACP);
}

char* gbk_to_utf8( const char * gbk)
{
    return str_convert(gbk,CP_ACP,CP_UTF8);
}
#else

#ifndef USE_ICONV
char* utf8_to_gbk(const char* utf)
{
	return 0;
}

char* gbk_to_utf8(const char* gbk)
{
	return 0;
}

char* utf8_to_gbk_2(const char* utf,int len)
{
	return 0;
}

char* gbk_to_utf8_2(const char* gbk,int len)
{
	return 0;
}

char* gbk_to_utf8_3(const char* gbk,int len)
{
	return 0;
}

char* utf8_to_gbk_3(const char *utf8,int len)
{
	return 0;
}
#else
/**
 * @return converted string bytes. if <0 is failed else success.
 */
int wtk_str_convert(const char* src,int src_bytes,const char* src_code,const char *dst_code,char *dst,int dst_bytes)
{
	iconv_t id;
	size_t in,out,c;
	int ret=-1;

	id=0;
	if(!src){goto end;}
	id=iconv_open(src_code,dst_code);
	if(!id){goto end;}
	in=src_bytes;
	out=dst_bytes;
	c=iconv(id,(char**)&src,&in,&dst,&out);
	//wtk_debug("c=%d,in=%d,out=%d\n",c,in,out);
	if(c<0){ret=-1;goto end;}
	ret=c;//dst_bytes-out;
	//wtk_debug("ret=%d\n",ret);
end:
	if(id){iconv_close(id);}
	return ret;
}

/**
 * @return chars must be freed.
 */
char* wtk_str_convert2(const char* src,int len,const char* src_code,const char *dst_code)
{
	int out,ret;
	char *dst;

	out=len*3+1;
	dst=(char*)wtk_calloc(1,out);
	ret=wtk_str_convert(src,len,src_code,dst_code,dst,out);
	if(ret<0)
	{
		wtk_free(dst);
		dst=0;
	}
	return dst;
}

/**
 * @return chars is in static buffer,do not need freed.
 */
char* wtk_str_convert3(const char* src,int len,const char* src_code,const char *dst_code)
{
#define N 1024
	static char buf[N];
	char *p;
	int ret;

	p=buf;
	memset(buf,0,N);
	ret=wtk_str_convert(src,len,src_code,dst_code,p,N);
	p=ret<0?0:buf;
	return p;
}

char* utf8_to_gbk(const char* utf)
{
	return utf8_to_gbk_2(utf,strlen(utf));
}

char* utf8_to_gbk_2(const char* utf,int len)
{
	return wtk_str_convert2(utf,len,"gb18030","utf-8");
}

char* gbk_to_utf8(const char* gbk)
{
	return gbk_to_utf8_2(gbk,strlen(gbk));
}

char* gbk_to_utf8_2(const char* gbk,int len)
{
	return wtk_str_convert2(gbk,len,"utf-8","gb18030");
}

char* gbk_to_utf8_3(const char* gbk,int len)
{
	return wtk_str_convert3(gbk,len,"utf-8","gb18030");
}

char* utf8_to_gbk_3(const char *utf8,int len)
{
	return wtk_str_convert3(utf8,len,"gb18030","utf-8");
}
#endif
#endif

int str_is_utf8(const unsigned char* utf,int len)
{
    unsigned char c;
    int i,count;
    int ret;

    count=0;ret=0;
    for(i=0;i<len;++i)
    {
        c=*(utf+i);
        if(c<0x80 && count==0){continue;}
        if(count==0)
        {
            if(c>=0xF0)
            {
                count=3;
            }else if(c>=0xE0)
            {
                count=2;
            }else if(c>=0x80)
            {
                count=1;
            }else
            {
                goto end;
            }
        }else
        {
            if((c&0xc0)!=0x80)
            {
                goto end;
            }
            --count;
        }
    }
    ret=1;
end:
     return ret;
}

int wtk_utf8_bytes(char c)
{
	/* UTF-8							*/
	/* 0000 0000-0000 007F | 0xxxxxxx				*/
	/* 0000 0080-0000 07FF | 110xxxxx 10xxxxxx			*/
	/* 0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx		*/
	/* 0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx	*/
	int num;

	if(c&0x80)
	{
		num=2;
		c=c<<2;
		while(c & 0x80)
		{
			c=c<<1;
			++num;
		}
	}else
	{
		num=1;
	}
	return num;
}

int wtk_utf8_len(char *data,int len)
{
	char *s,*e;
	int count=0;
	int t;

	s=data;e=s+len;
	while(s<e)
	{
		t=wtk_utf8_bytes(*s);
		++count;
		s+=t;
	}
	return count;
}


void wtk_utf8_tolower(char *data,int len)
{
	char *s,*e;
	int t;

	s=data;e=s+len;
	while(s<e)
	{
		t=wtk_utf8_bytes(*s);
		if(t==1 && isupper(*s))
		{
			//wtk_debug("tolower\n");
			*s=tolower(*s);
		}
		s+=t;
	}
}

int wtk_utf16_to_utf8(int v,char *buf)
{
	int len;
	//case 3: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
	//case 2: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
	/* UTF-8							*/
	/* 0000 0000-0000 007F | 0xxxxxxx				*/
	/* 0000 0080-0000 07FF | 110xxxxx 10xxxxxx			*/
	/* 0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx		*/
	/* 0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx	*/
	//B=11   1011
	//A=10	 1010
	if(v<0x80)
	{
		len=1;
		buf[0]=v;
	}else if(v<0x800)
	{
		len=2;
		buf[1]=((v|0x80)&0xBF);
		buf[0]=(v>>6)|0xC0;
	}else
	{
		len=3;
		buf[2]=((v|0x80)&0xBF);
		buf[1]=((v>>6)|0x80)&0xBF;
		buf[0]=(v>>12)|0xE0;
	}
	//wtk_debug("[%.*s]\n",len,buf);
	return len;
}

int wtk_utf8_to_utf16(char *data)
{
	int bytes;
	int v;

	bytes=wtk_utf8_bytes(*data);
	switch(bytes)
	{
	case 1:
		v=*(data);
		break;
	case 2:
		v=(data[0]&0x1F)<<6;
		v+=(data[1]&0x3F);
		break;
	case 3:
		v=(data[0]&0x0F)<<12;
		v+=(data[1]&0x3f)<<6;
		v+=(data[2]&0x3f);
		break;
	default:
		v=0;
		break;
	}
	return v;
}


