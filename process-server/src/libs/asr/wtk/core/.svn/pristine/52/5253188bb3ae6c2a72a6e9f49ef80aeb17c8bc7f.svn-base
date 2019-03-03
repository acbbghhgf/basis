#include "wtk_gzip.h"
void wtk_gzip_zip_finish(wtk_gzip_t *zip);

wtk_gzip_t* wtk_gzip_new()
{
	wtk_gzip_t *gz;

	gz=(wtk_gzip_t*)wtk_malloc(sizeof(wtk_gzip_t));
	gz->buf=wtk_strbuf_new(256,1);
	gz->cache_size=4096;
	//gz->cache_size=256;
	gz->cache=(char*)wtk_malloc(gz->cache_size);
	return gz;
}

void wtk_gzip_delete(wtk_gzip_t *gz)
{
	wtk_free(gz->cache);
	wtk_strbuf_delete(gz->buf);
	wtk_free(gz);
}


static void storLE32 (uint8_t *p, uint32_t n) {
	p[0] = n >>  0;
	p[1] = n >>  8;
	p[2] = n >> 16;
	p[3] = n >> 24;
}

void wtk_gzip_zip_start(wtk_gzip_t *gz)
{
	uint8_t hdr[10] = {
		0x1F, 0x8B,	/* magic */
		8,		/* z method */
		0,		/* flags */
		0,0,0,0,	/* mtime */
		0,		/* xfl */
		0xFF,		/* OS */
	};
	wtk_strbuf_t *buf=gz->buf;
	z_stream *s=&(gz->stream);
	int windowBits = -MZ_DEFAULT_WINDOW_BITS;

	wtk_strbuf_reset(buf);
	wtk_strbuf_push(buf,(char*)hdr,10);
	memset(s,0,sizeof(z_stream));
	deflateInit2 (s,6,MZ_DEFLATED, windowBits, 6, MZ_DEFAULT_STRATEGY);
}

void wtk_gzip_zip_write(wtk_gzip_t *gz,char *data,int bytes,int is_end)
{
	wtk_strbuf_t *buf=gz->buf;
	z_stream *s=&(gz->stream);
	int st;
	int v;
	int b;

	//wtk_debug("======================> in=%d is_end=%d\n",bytes,is_end);
	s->next_in=(unsigned char*)data;
	s->avail_in =bytes;
	s->next_out  = (unsigned char*)gz->cache;
	s->avail_out = gz->cache_size;
	b=1;
	while(b)
	{
		st=mz_deflate (s,is_end?MZ_FINISH:MZ_NO_FLUSH);//bytes>0?MZ_NO_FLUSH:MZ_FINISH);
		//bytes=0;
		//wtk_debug("st=%d/%d bytes=%d avin=%d avout=%d\n",st,MZ_OK,bytes,s->avail_in,s->avail_out);
		switch(st)
		{
		case MZ_STREAM_END:
			//fall through
			b=0;
		case MZ_OK:
			//wtk_debug("out=%d b=%d\n",s->avail_out,b);
			v=gz->cache_size-s->avail_out;
			//wtk_debug("v=%d\n",v);
			if(v<gz->cache_size)//s->avail_out>0)
			{
				b=0;
			}
			if(v>0)
			{
				wtk_strbuf_push(buf,gz->cache,v);
				s->next_out  = (unsigned char*)gz->cache;
				s->avail_out = gz->cache_size;
			}
			//s->avail_in=0;
			//b=0;
			break;
		case MZ_BUF_ERROR:
			wtk_debug("buf error\n");
			b=0;
			break;
		case MZ_DATA_ERROR:
			wtk_debug("data error\n");
			b=0;
			break;
		case MZ_PARAM_ERROR:
			wtk_debug("param error\n");
			b=0;
			break;
		case MZ_STREAM_ERROR:
			wtk_debug("stream error\n");
			b=0;
			break;
		}
	}
	//wtk_debug("size=%d\n",buf->pos);
	//exit(0);
	if(is_end)
	{
		wtk_gzip_zip_finish(gz);
	}
}

void wtk_gzip_zip_finish(wtk_gzip_t *gz)
{
	uint8_t ftr[8];
	wtk_strbuf_t *buf=gz->buf;

	gz->st.l=gz->stream.total_in;
	gz->st.crc32=gz->stream.crc32;
	storLE32 (ftr + 0, gz->st.crc32);
	storLE32 (ftr + 4, gz->st.l);
	wtk_strbuf_push(buf,(char*)ftr,8);
	//wtk_debug("size=%d\n",buf->pos);
	//exit(0);
	deflateEnd(&(gz->stream));
}


void wtk_gzip_unzip_start(wtk_gzip_t *gz)
{
	wtk_strbuf_t *buf=gz->buf;
	z_stream *s=&(gz->stream);
	int windowBits = -MZ_DEFAULT_WINDOW_BITS;

	wtk_strbuf_reset(buf);
	memset(s,0,sizeof(z_stream));
	inflateInit2(s,windowBits);
	gz->state=WTK_GZIP_UNZIP_INIT;
}


int wtk_gzip_unzip_read(wtk_gzip_t *gz,char *data,int bytes,int is_end)
{
	wtk_strbuf_t *buf=gz->buf;
	int n;
	int ret;
	z_stream *s=&(gz->stream);
	int st;
	int v;
	int b;

	//wtk_debug("====================== bytes=%d ==================\n",bytes);
	switch(gz->state)
	{
	case WTK_GZIP_UNZIP_INIT:
		n=min(10-buf->pos,bytes);
		wtk_strbuf_push(buf,data,n);
		if(buf->pos==10)
		{
			if(buf->data[0]!=0x1f || buf->data[1]==0x8b)
			{
				wtk_debug("not in gz format");
				ret=-1;
				goto end;
			}
			if(buf->data[2]!=8)
			{
				wtk_debug("unknown z-algorithm: 0x%0hhX", buf->data[2]);
				ret=-1;
				goto end;
			}
			gz->hdr_flags=buf->data[3];
			if (gz->hdr_flags & (1 << 2))
			{
				gz->state=WTK_GZIP_UNZIP_FLAG_EXTRA;
			}else if(gz->hdr_flags & (1<<3))
			{
				gz->state=WTK_GZIP_UNZIP_FLAG_FNAME;
			}else if(gz->hdr_flags & (1<<4))
			{
				gz->state=WTK_GZIP_UNZIP_FLAG_FCOMMENT;
			}else if(gz->hdr_flags & (1<<1))
			{
				gz->state=WTK_GZIP_UNZIP_FLAG_FCRC;
			}else
			{
				gz->state=WTK_GZIP_UNZIP_RUN;
			}
			wtk_strbuf_reset(buf);
			bytes-=n;
			if(bytes>0)
			{
				return wtk_gzip_unzip_read(gz,data+n,bytes,is_end);
			}
		}
		break;
	case WTK_GZIP_UNZIP_FLAG_EXTRA:
		n=min(2-buf->pos,bytes);
		wtk_strbuf_push(buf,data,n);
		if(buf->pos==2)
		{
			gz->state=WTK_GZIP_UNZIP_FLAG_EXTRA_SKIP;
			gz->next_bytes=(buf->data[1]<<8)+buf->data[0];
			wtk_strbuf_reset(buf);
			bytes-=n;
			if(bytes>0)
			{
				return wtk_gzip_unzip_read(gz,data+n,bytes,is_end);
			}
		}
		break;
	case WTK_GZIP_UNZIP_FLAG_EXTRA_SKIP:
		n=min(gz->next_bytes-buf->pos,bytes);
		wtk_strbuf_push(buf,data,n);
		if(buf->pos==gz->next_bytes)
		{
			if(gz->hdr_flags & (1<<3))
			{
				gz->state=WTK_GZIP_UNZIP_FLAG_FNAME;
			}else if(gz->hdr_flags & (1<<4))
			{
				gz->state=WTK_GZIP_UNZIP_FLAG_FCOMMENT;
			}else if(gz->hdr_flags & (1<<1))
			{
				gz->state=WTK_GZIP_UNZIP_FLAG_FCRC;
			}else
			{
				gz->state=WTK_GZIP_UNZIP_RUN;
			}
			wtk_strbuf_reset(buf);
			bytes-=n;
			if(bytes>0)
			{
				return wtk_gzip_unzip_read(gz,data+n,bytes,is_end);
			}
		}
		break;
	case WTK_GZIP_UNZIP_FLAG_FNAME:
		for(n=0;n<bytes;++n)
		{
			if(data[n]==0)
			{
				if(gz->hdr_flags & (1<<4))
				{
					gz->state=WTK_GZIP_UNZIP_FLAG_FCOMMENT;
				}else if(gz->hdr_flags & (1<<1))
				{
					gz->state=WTK_GZIP_UNZIP_FLAG_FCRC;
				}else
				{
					gz->state=WTK_GZIP_UNZIP_RUN;
				}
				wtk_strbuf_reset(buf);
				++n;
				bytes-=n;
				if(bytes>0)
				{
					return wtk_gzip_unzip_read(gz,data+n,bytes,is_end);
				}
				break;
			}
		}
		break;
	case WTK_GZIP_UNZIP_FLAG_FCOMMENT:
		for(n=0;n<bytes;++n)
		{
			if(data[n]==0)
			{
				if(gz->hdr_flags & (1<<1))
				{
					gz->state=WTK_GZIP_UNZIP_FLAG_FCRC;
				}else
				{
					gz->state=WTK_GZIP_UNZIP_RUN;
				}
				wtk_strbuf_reset(buf);
				++n;
				bytes-=n;
				if(bytes>0)
				{
					return wtk_gzip_unzip_read(gz,data+n,bytes,is_end);
				}
				break;
			}
		}
		break;
	case WTK_GZIP_UNZIP_FLAG_FCRC:
		n=min(2-buf->pos,bytes);
		wtk_strbuf_push(buf,data,n);
		if(buf->pos==2)
		{
			gz->state=WTK_GZIP_UNZIP_RUN;
			wtk_strbuf_reset(buf);
			bytes-=n;
			if(bytes>0)
			{
				return wtk_gzip_unzip_read(gz,data+n,bytes,is_end);
			}
		}
		break;
	case WTK_GZIP_UNZIP_RUN:
		s->next_in=(unsigned char*)data;
		s->avail_in=bytes;
		s->next_out  = (unsigned char*)gz->cache;
		s->avail_out = gz->cache_size;
		b=1;
		while(b)
		{
			st=mz_inflate(s,MZ_SYNC_FLUSH);
			switch(st)
			{
			case MZ_STREAM_END:
				//fall through
				b=0;
			case MZ_OK:
				//wtk_debug("out=%d b=%d\n",s->avail_out,b);
				v=gz->cache_size-s->avail_out;
				//wtk_debug("out=%d out=%d in=%d pos=%d\n",v,s->avail_out,s->avail_in,buf->pos);
				if(v<gz->cache_size)
				{
					b=0;
				}
				if(v>0)
				{
					wtk_strbuf_push(buf,gz->cache,v);
					s->next_out  = (unsigned char*)gz->cache;
					s->avail_out = gz->cache_size;
				}
				//s->avail_in=0;
				//b=0;
				break;
			case MZ_BUF_ERROR:
				wtk_debug("buf error\n");
				b=0;
				break;
			case MZ_DATA_ERROR:
				wtk_debug("data error\n");
				b=0;
				break;
			case MZ_PARAM_ERROR:
				wtk_debug("param error\n");
				b=0;
				break;
			case MZ_STREAM_ERROR:
				wtk_debug("stream error\n");
				b=0;
				break;
			}
		}
		//exit(0);
		break;
	}
	ret=0;
end:
	if(is_end)
	{
		inflateEnd(s);
	}
	return ret;
}

