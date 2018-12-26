#include "wtk_base64.h" 

const char* base64char="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* wtk_base64_encode(char* data,int len)
{
	 int i, j;
	unsigned char current;
	char *base64;


	base64=(char*)wtk_malloc(len*2);
	memset(base64,0,len*2);
	for ( i = 0, j = 0 ; i < len ; i += 3 )
	{
		current = (data[i] >> 2) ;
		current &= (unsigned char)0x3F;
		base64[j++] = base64char[(int)current];

		current = ( (unsigned char)(data[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
		if ( i + 1 >= len )
		{
			base64[j++] = '=';
			base64[j++] = '=';
			break;
		}
		current |= ( (unsigned char)(data[i+1] >> 4) ) & ( (unsigned char) 0x0F );
		base64[j++] = base64char[(int)current];

		current = ( (unsigned char)(data[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
		if ( i + 2 >= len )
		{
			base64[j++] = base64char[(int)current];
			base64[j++] = '=';
			break;
		}
		current |= ( (unsigned char)(data[i+2] >> 6) ) & ( (unsigned char) 0x03 );
		base64[j++] = base64char[(int)current];

		current = ( (unsigned char)data[i+2] ) & ( (unsigned char)0x3F ) ;
		base64[j++] = base64char[(int)current];
	}
	wtk_debug("%d\n",j);
	base64[j] = '\0';
	return base64;
}

#define CONVERT_PLUS(base, idx, c)                \
    if (c == '+') { \
    base[idx++] = '%'; \
    base[idx++] = '2'; \
    base[idx++] = 'b'; \
    } else { \
    base[idx++] = c; \
    }

char* wtk_base64_encode_url(char* data,int len)
{
	 int i, j;
	unsigned char current;
	char *base64;


	base64=(char*)wtk_malloc(len*2);
	memset(base64,0,len*2);
	for ( i = 0, j = 0 ; i < len ; i += 3 )
	{
		current = (data[i] >> 2) ;
		current &= (unsigned char)0x3F;
		CONVERT_PLUS(base64, j, base64char[(int)current]);

		current = ( (unsigned char)(data[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
		if ( i + 1 >= len )
		{
			base64[j++] = '=';
			base64[j++] = '=';
			break;
		}
		current |= ( (unsigned char)(data[i+1] >> 4) ) & ( (unsigned char) 0x0F );
		CONVERT_PLUS(base64, j, base64char[(int)current]);

		current = ( (unsigned char)(data[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
		if ( i + 2 >= len )
		{
			base64[j++] = base64char[(int)current];
			base64[j++] = '=';
			break;
		}
		current |= ( (unsigned char)(data[i+2] >> 6) ) & ( (unsigned char) 0x03 );
		CONVERT_PLUS(base64, j, base64char[(int)current]);

		current = ( (unsigned char)data[i+2] ) & ( (unsigned char)0x3F ) ;
		CONVERT_PLUS(base64, j, base64char[(int)current]);
	}
	base64[j] = '\0';
	return base64;
}

char* wtk_base64_decode(const char* base64,int len)
{
	int i, j;
	unsigned char k;
	unsigned char temp[4];
	char *data;

	data=(char*)wtk_malloc(len);
	for ( i = 0, j = 0; base64[i] != '\0' ; i += 4 )
	{
		memset( temp, 0xFF, sizeof(temp) );
		for ( k = 0 ; k < 64 ; k ++ )
		{
			if ( base64char[k] == base64[i] )
				temp[0]= k;
		}
		for ( k = 0 ; k < 64 ; k ++ )
		{
			if ( base64char[k] == base64[i+1] )
				temp[1]= k;
		}
		for ( k = 0 ; k < 64 ; k ++ )
		{
			if ( base64char[k] == base64[i+2] )
				temp[2]= k;
		}
		for ( k = 0 ; k < 64 ; k ++ )
		{
			if ( base64char[k] == base64[i+3] )
				temp[3]= k;
		}

		data[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2))&0xFC)) |
			((unsigned char)((unsigned char)(temp[1]>>4)&0x03));
		if ( base64[i+2] == '=' )
			break;

		data[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4))&0xF0)) |
			((unsigned char)((unsigned char)(temp[2]>>2)&0x0F));
		if ( base64[i+3] == '=' )
			break;

		data[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6))&0xF0)) |
			((unsigned char)(temp[3]&0x3F));
	    }
	    data[j] = 0;
	    return data;
}
