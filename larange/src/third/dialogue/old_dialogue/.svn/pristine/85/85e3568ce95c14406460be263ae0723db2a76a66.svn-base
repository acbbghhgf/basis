#include<stdio.h>
#include"qtk/https/qtk_https.h"
//现在只支持get方法 post方法

qtk_https_t *qtk_https_new(void)
{
	qtk_https_t *https = NULL;

	https = malloc(sizeof(qtk_https_t));

	memset(https,0,sizeof(qtk_https_t));

	https->rbuf = wtk_strbuf_new(1024,2.0);
	curl_global_init(CURL_GLOBAL_DEFAULT);//这里可能出问题
	//curl_global_init(CURL_GLOBAL_ALL);
	https->curl = curl_easy_init();
	
	return https;
}


static int qtk_write_data_callback(void *buf, size_t size, size_t nmemb, void *stream)
{
	int len = size*nmemb;
	wtk_strbuf_t *str_buf = (wtk_strbuf_t*)stream;
	
	wtk_strbuf_push(str_buf,(char*)buf,len);

	return len;
}

//https的get方法
int qtk_https_get(qtk_https_t *https)
{
	int ret = 0;
	CURLcode res;
	wtk_strbuf_t *buf = NULL;

	if(!https->curl){
		ret = -1;
		goto end;
	}
	
	buf = wtk_strbuf_new(1024,1.2);

	//需要有https头
	if(https->host.len){
		wtk_strbuf_push_string(buf,"https://");
		wtk_strbuf_push(buf,https->host.data,https->host.len);
	}else{
		goto end;
	}
	if(https->port.len){
		wtk_strbuf_push_string(buf,":");
		wtk_strbuf_push(buf,https->port.data,https->port.len);
	}
	if(https->url.len){
		wtk_strbuf_push(buf,https->url.data,https->url.len);
	}
	//放一个\0到最后
	wtk_strbuf_push_s0(buf,'\0');
	if(https->curl){
		curl_easy_setopt(https->curl,CURLOPT_URL,buf->data);
		curl_easy_setopt(https->curl,CURLOPT_WRITEFUNCTION,qtk_write_data_callback);
		curl_easy_setopt(https->curl,CURLOPT_WRITEDATA,https->rbuf);
		//不验证服务器提供的证书
		curl_easy_setopt(https->curl,CURLOPT_SSL_VERIFYPEER,0L);
		curl_easy_setopt(https->curl,CURLOPT_SSL_VERIFYHOST,0L);
		//设置output超时
		if(https->timeout > 0){
			curl_easy_setopt(https->curl,CURLOPT_TIMEOUT_MS,https->timeout);
			curl_easy_setopt(https->curl,CURLOPT_NOSIGNAL,1);
		}	
		res=curl_easy_perform(https->curl);
		if(res != CURLE_OK){
			ret = -1;
			wtk_debug("curl_easy_perform failed:%s\n",curl_easy_strerror(res));
		}
		
	}
	
end:
	if(buf){
		wtk_strbuf_delete(buf);
	}
	return ret;
}

int qtk_https_post(qtk_https_t *https, wtk_string_t *postthis)
{
	int ret = 0;
	CURLcode res;
	wtk_strbuf_t *buf = NULL;
	struct curl_slist *headers = NULL;

	if(!https->curl){
		ret = -1;
		goto end;
	}
	
	buf = wtk_strbuf_new(1024,1.2);

	//需要有https host
	if(https->host.len){
		wtk_strbuf_push_string(buf,"https://");
		wtk_strbuf_push(buf,https->host.data,https->host.len);
	}else{
		goto end;
	}
	if(https->port.len){
		wtk_strbuf_push_string(buf,":");
		wtk_strbuf_push(buf,https->port.data,https->port.len);
	}
	if(https->url.len){
		wtk_strbuf_push(buf,https->url.data,https->url.len);
	}
	//放一个\0到最后
	wtk_strbuf_push_s0(buf,'\0');

	//设置http请求头
	headers = curl_slist_append(headers,"Accept:application/json");
	headers = curl_slist_append(headers, "Content-Type:application/x-www-form-urlencoded");
	curl_easy_setopt(https->curl, CURLOPT_HTTPHEADER, headers);

	if(https->curl){
		//curl_easy_setopt(https->curl,CURLOPT_VERBOSE,1);
		//curl_easy_setopt(https->curl, CURLOPT_HEADER, 1);

		curl_easy_setopt(https->curl,CURLOPT_URL,buf->data);
		curl_easy_setopt(https->curl,CURLOPT_WRITEFUNCTION,qtk_write_data_callback);
		curl_easy_setopt(https->curl,CURLOPT_WRITEDATA,https->rbuf);
		//不验证服务器提供的证书
		curl_easy_setopt(https->curl,CURLOPT_SSL_VERIFYPEER,0L);
		curl_easy_setopt(https->curl,CURLOPT_SSL_VERIFYHOST,0L);
		//设置output超时
		if(https->timeout > 0){
			curl_easy_setopt(https->curl,CURLOPT_TIMEOUT_MS,https->timeout);
			curl_easy_setopt(https->curl,CURLOPT_NOSIGNAL,1);
		}
		curl_easy_setopt(https->curl,CURLOPT_POSTFIELDSIZE,postthis->len);
		curl_easy_setopt(https->curl,CURLOPT_POSTFIELDS,postthis->data);

		res=curl_easy_perform(https->curl);
		if(res != CURLE_OK){
			wtk_debug("curl_easy_perform failed:%s\n",curl_easy_strerror(res));
			goto end;
		}
	}
	ret = 1;
end:
	if(buf){
		wtk_strbuf_delete(buf);
	}
	return ret;
}

void qtk_https_delete(qtk_https_t *https)
{
	if(https->curl){
		curl_easy_cleanup(https->curl);
	}
	curl_global_cleanup();
	wtk_strbuf_delete(https->rbuf);
	return;
}
