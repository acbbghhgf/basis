#include "wtk_gbk.h" 
#include "gbk.h"


unsigned int convert_to_gbk(unsigned int uni_code) {
    const unsigned short *page = from_uni[(uni_code >> 8) & 0xFF];
    return page == NULL ? 0x3F : page[uni_code & 0xFF];
}

unsigned int convert_from_gbk(unsigned int gbk_code) {
    const unsigned short *page = to_uni[(gbk_code >> 8) & 0xFF];
    return page == NULL ? 0xFFFE : page[gbk_code & 0xFF];
}

size_t utf8_encode(char *s, unsigned int ch) {
    if (ch < 0x80) {
        s[0] = (char)ch;
        return 1;
    }
    if (ch <= 0x7FF) {
        s[1] = (char) ((ch | 0x80) & 0xBF);
        s[0] = (char) ((ch >> 6) | 0xC0);
        return 2;
    }
    if (ch <= 0xFFFF) {
three:
        s[2] = (char) ((ch | 0x80) & 0xBF);
        s[1] = (char) (((ch >> 6) | 0x80) & 0xBF);
        s[0] = (char) ((ch >> 12) | 0xE0);
        return 3;
    }
    if (ch <= 0x1FFFFF) {
        s[3] = (char) ((ch | 0x80) & 0xBF);
        s[2] = (char) (((ch >> 6) | 0x80) & 0xBF);
        s[1] = (char) (((ch >> 12) | 0x80) & 0xBF);
        s[0] = (char) ((ch >> 18) | 0xF0);
        return 4;
    }
    if (ch <= 0x3FFFFFF) {
        s[4] = (char) ((ch | 0x80) & 0xBF);
        s[3] = (char) (((ch >> 6) | 0x80) & 0xBF);
        s[2] = (char) (((ch >> 12) | 0x80) & 0xBF);
        s[1] = (char) (((ch >> 18) | 0x80) & 0xBF);
        s[0] = (char) ((ch >> 24) | 0xF8);
        return 5;
    }
    if (ch <= 0x7FFFFFFF) {
        s[5] = (char) ((ch | 0x80) & 0xBF);
        s[4] = (char) (((ch >> 6) | 0x80) & 0xBF);
        s[3] = (char) (((ch >> 12) | 0x80) & 0xBF);
        s[2] = (char) (((ch >> 18) | 0x80) & 0xBF);
        s[1] = (char) (((ch >> 24) | 0x80) & 0xBF);
        s[0] = (char) ((ch >> 30) | 0xFC);
        return 6;
    }

    /* fallback */
    ch = 0xFFFD;
    goto three;
}

size_t utf8_decode(const char *s, const char *e, unsigned int *pch) {
    unsigned int ch;

    if (s >= e) {
        *pch = 0;
        return 0;
    }

    ch = (unsigned char)s[0];
    if (ch < 0xC0) goto fallback;
    if (ch < 0xE0) {
        if (s+1 >= e || (s[1] & 0xC0) != 0x80)
            goto fallback;
        *pch = ((ch   & 0x1F) << 6) |
            (s[1] & 0x3F);
        return 2;
    }
    if (ch < 0xF0) {
        if (s+2 >= e || (s[1] & 0xC0) != 0x80
                || (s[2] & 0xC0) != 0x80)
            goto fallback;
        *pch = ((ch   & 0x0F) << 12) |
            ((s[1] & 0x3F) <<  6) |
            (s[2] & 0x3F);
        return 3;
    }
    {
        int count = 0; /* to count number of continuation bytes */
        unsigned int res;
        while ((ch & 0x40) != 0) { /* still have continuation bytes? */
            int cc = (unsigned char)s[++count];
            if ((cc & 0xC0) != 0x80) /* not a continuation byte? */
                goto fallback; /* invalid byte sequence, fallback */
            res = (res << 6) | (cc & 0x3F); /* add lower 6 bits from cont. byte */
            ch <<= 1; /* to test next bit */
        }
        if (count > 5)
            goto fallback; /* invalid byte sequence */
        res |= ((ch & 0x7F) << (count * 5)); /* add first byte */
        return count+1;
    }

fallback:
    *pch = ch;
    return 1;
}

//static void add_utf8char(luaL_Buffer *b, unsigned int ch) {
//    char buff[UTF_MAX];
//    size_t n = utf8_encode(buff, ch);
//    luaL_addlstring(b, buff, n);
//}

size_t gbk_decode(const char *s, const char *e, unsigned *pch) {
    unsigned int ch;
    if (s >= e) {
        *pch = 0;
        return 0;
    }

    ch = s[0] & 0xFF;
    if (ch < 0x7F) {
        *pch = ch;
        return 1;
    }

    *pch = (ch << 8) | (s[1] & 0xFF);
    return 2;
}


size_t gbk_length(const char *s, const char *e) {
    size_t gbklen = 0;
    while (s < e) {
        if ((unsigned char)(*s++) > 0x7F)
            ++s;
        ++gbklen;
    }
    return gbklen;
}

void wtk_gbk_buf_add_char(wtk_strbuf_t *buf,unsigned int ch)
{
	if(ch<0x7f)
	{
		wtk_strbuf_push_c(buf,ch&0xFF);
	}else
	{
		wtk_strbuf_push_c(buf,(ch>>8)&0xFF);
		wtk_strbuf_push_c(buf,ch&0xFF);
	}
}


char* wtk_utf8_to_gbk(char *s,int len)
{
	unsigned int ch=0;
	char *e;
	wtk_strbuf_t *buf;
	char *v;

	buf=wtk_strbuf_new(256,1);
	e=s+len;
    while (s < e)
    {
        s += utf8_decode(s, e, &ch);
        wtk_gbk_buf_add_char(buf,convert_to_gbk(ch));
    }
    v=wtk_data_to_str(buf->data,buf->pos);
    wtk_strbuf_delete(buf);
    return v;
}


//static int gbk_string_to_utf8(lua_State *L) {
//    const char *e, *s = check_gbk(L, 1, &e);
//    luaL_Buffer b;
//    luaL_buffinit(L, &b);
//    while (s < e) {
//        unsigned int ch;
//        s += gbk_decode(s, e, &ch);
//        add_utf8char(&b, convert_from_gbk(ch));
//    }
//    luaL_pushresult(&b);
//    return 1;
//}


