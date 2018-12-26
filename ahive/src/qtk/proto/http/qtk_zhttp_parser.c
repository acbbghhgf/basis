#include "qtk_zhttp_parser.h"
#include "qtk/core/qtk_deque.h"
#include "qtk/zeus/qtk_zeus_server.h"
#include "ctype.h"

#define HTTP_STATUS_MAP(no,s) \
	case no: return s;


const char *qtk_zhttp_status_str(int status)
{
    switch (status) {
	HTTP_STATUS_MAP(100, "Continue");
	HTTP_STATUS_MAP(101, "witching Protocols");
	HTTP_STATUS_MAP(200, "OK");
	HTTP_STATUS_MAP(201, "Created");
	HTTP_STATUS_MAP(202, "Accepted");
	HTTP_STATUS_MAP(203, "Non-Authoritative Information");
	HTTP_STATUS_MAP(204, "No Content");
	HTTP_STATUS_MAP(205, "Reset Content");
	HTTP_STATUS_MAP(206, "Partial Content");
	HTTP_STATUS_MAP(300, "Multiple Choices");
	HTTP_STATUS_MAP(301, "Moved Permanently");
	HTTP_STATUS_MAP(302, "Found");
	HTTP_STATUS_MAP(303, "See Other");
	HTTP_STATUS_MAP(304, "Not Modified");
	HTTP_STATUS_MAP(305, "Use Proxy");
	HTTP_STATUS_MAP(307, "Temporary Redirect");
	HTTP_STATUS_MAP(400, "Bad Request");
	HTTP_STATUS_MAP(401, "Unauthorized");
	HTTP_STATUS_MAP(402, "Payment Required");
	HTTP_STATUS_MAP(403, "Forbidden");
	HTTP_STATUS_MAP(404, "Not Found");
	HTTP_STATUS_MAP(405, "Method Not Allowed");
	HTTP_STATUS_MAP(406, "Not Acceptable");
	HTTP_STATUS_MAP(407, "Proxy Authentication Required");
	HTTP_STATUS_MAP(408, "Request Time-out");
	HTTP_STATUS_MAP(409, "Conflict");
	HTTP_STATUS_MAP(410, "Gone");
	HTTP_STATUS_MAP(411, "Length Required");
	HTTP_STATUS_MAP(412, "Precondition Failed");
	HTTP_STATUS_MAP(413, "Request Entity Too Large");
	HTTP_STATUS_MAP(414, "Request-URI Too Large");
	HTTP_STATUS_MAP(415, "Unsupported Media Type");
	HTTP_STATUS_MAP(416, "Requested range not satisfiable");
	HTTP_STATUS_MAP(417, "Expectation Failed");
	HTTP_STATUS_MAP(500, "Internal Server Error");
	HTTP_STATUS_MAP(501, "Not Implemented");
	HTTP_STATUS_MAP(502, "Bad Gateway");
	HTTP_STATUS_MAP(503, "Service Unavailable");
	HTTP_STATUS_MAP(504, "Gateway Time-out");
	HTTP_STATUS_MAP(505, "HTTP Version not supported");
    }
    return NULL;
}


static void qtk_http_parser_save_key(qtk_zhttp_parser_t *p, char *data, int bytes) {
    qtk_httphdr_set_string(p->pack, &p->key, data, bytes);
}


static int qtk_parser_process_body(qtk_zhttp_parser_t *p, qtk_deque_t *dq) {
    if (dq->len >= p->pack->content_length) {
        p->state = HTTP_CONTENT_DONE;
        return 1;
    }
    return 0;
}


static int qtk_http_parser_feed_request(qtk_zhttp_parser_t *p, qtk_deque_t *dq) {
	wtk_strbuf_t *buf = p->tmp_buf;
    qtk_http_hdr_t *r = p->pack;
	char ch;
	int ret = 0;

	while (dq->len) {
		qtk_deque_pop(dq, &ch, 1);
		switch (p->state) {
		case HTTP_METHOD:
			if (ch == ' ') {
				if ((buf->pos==4) && ngx_str4_cmp(buf->data,'P','O','S','T')) {
					r->req.method=HTTP_POST;
				} else if((buf->pos==3) && ngx_str3_cmp(buf->data,'G','E','T')) {
					r->req.method=HTTP_GET;
				} else if((buf->pos==3) && ngx_str3_cmp(buf->data,'P','U','T')) {
					r->req.method=HTTP_PUT;
				} else if((buf->pos==4) && ngx_str4_cmp(buf->data,'H','E','A','D')) {
					r->req.method=HTTP_HEAD;
				} else if((buf->pos==6) && !memcmp(buf->data, "DELETE", 6)) {
					r->req.method=HTTP_DELETE;
				} else {
					wtk_debug("only POST,GET,PUT,DELET,HEAD method is support now\r\n");
					wtk_debug("the method is %.*s\r\n", buf->pos, buf->data);
                    qtk_zserver_log("method \"%.*s\" is not support", buf->pos, buf->data);
					ret = -1;
					goto end;
				}
				p->state = HTTP_URL_SPACE;
			} else {
				wtk_strbuf_push_c(buf,ch);
			}
			break;
		case HTTP_URL_SPACE:
			if (ch != ' ') {
				wtk_strbuf_reset(buf);
				wtk_strbuf_push_c(buf,ch);
				p->state = HTTP_URL;
			}
			break;
		case HTTP_URL:
			if (ch == ' ' || ch == '?') {
				qtk_httphdr_set_string(r, &r->req.url, buf->data, buf->pos);
				if (ch == ' ') {
					p->state = HTTP_WAIT_VERSION;
				} else {
					wtk_strbuf_reset(buf);
					p->state = HTTP_URL_PARAM;
				}
			} else {
				wtk_strbuf_push_c(buf,ch);
			}
			break;
		case HTTP_URL_PARAM:
			if (ch == ' ') {
				qtk_httphdr_set_string(r, &r->req.params, buf->data, buf->pos);
				p->state = HTTP_WAIT_VERSION;
			} else {
				wtk_strbuf_push_c(buf,ch);
			}
			break;
		case HTTP_WAIT_VERSION:
			if (ch != ' ') {
				wtk_strbuf_reset(buf);
				wtk_strbuf_push_c(buf,ch);
				p->state = HTTP_VERSION;
			}
			break;
		case HTTP_VERSION:
			if (ch == '\r') {
				/* r->http1_1 = wtk_str_equal_s(buf->data, buf->pos, "HTTP/1.1"); */
				p->state = HTTP_CR;
			} else {
				wtk_strbuf_push_c(buf,ch);
			}
			break;
		case HTTP_CR:
			if (ch == '\n') {
				p->state = HTTP_CL;
			} else {
				wtk_debug("except '\n'\r\n");
				return -1;
			}
			break;
		case HTTP_CL:
			if (ch == '\r') {
				p->state = HTTP_HDR_ALMOST_DONE;
			} else {
				wtk_strbuf_reset(buf);
				wtk_strbuf_push_c(buf, tolower(ch));
				p->state = HTTP_KEY;
			}
			break;
		case HTTP_KEY:
			if (ch==':') {
				qtk_http_parser_save_key(p, buf->data, buf->pos);
				p->state = HTTP_KEY_SPACE;
			} else {
				wtk_strbuf_push_c(buf, tolower(ch));
			}
			break;
		case HTTP_KEY_SPACE:
			if (ch != ' ') {
				wtk_strbuf_reset(buf);
				wtk_strbuf_push_c(buf,ch);
				p->state = HTTP_VALUE;
			}
			break;
		case HTTP_VALUE:
			if(ch == '\r') {
				ret = qtk_httphdr_add(r, p->key.data, p->key.len, buf->data, buf->pos);
                if (ret) goto end;
				p->state = HTTP_CR;
			} else {
				wtk_strbuf_push_c(buf, ch);
			}
			break;
		case HTTP_HDR_ALMOST_DONE:
			if (ch == '\n') {
                wtk_strbuf_reset(buf);
                if (r->content_length == 0) {
                    p->state = HTTP_CONTENT_DONE;
                    ret = 1;
                } else {
                    p->state = HTTP_HDR_DONE;
                    ret = qtk_parser_process_body(p, dq);
                }
                goto end;
			}
			break;
		default:
			wtk_debug("ah:never be here.\n");
			ret = -1;
			goto end;
		}
	}
end:
	return ret;
}


static int qtk_http_parser_feed_response(qtk_zhttp_parser_t *p, qtk_deque_t *dq) {
    wtk_strbuf_t *buf = p->tmp_buf;
    qtk_http_hdr_t *rep = p->pack;
    char ch;
    int ret = 0;

	while (dq->len) {
		qtk_deque_pop(dq, &ch, 1);
		switch (p->state) {
		case HTTP_VERSION:
			//HTTP/1.1 #200 OK
			if (ch == ' ') {
				//seek to 200
				wtk_strbuf_reset(buf);
				p->state = HTTP_STATUS_CODE;
			}
			break;
		case HTTP_STATUS_CODE:
			if (ch == ' ') {
                qtk_httphdr_set_repstaus(rep, buf->data, buf->pos);
				p->state = HTTP_STATUS_INFO;
			} else {
				wtk_strbuf_push_c(buf, ch);
			}
			break;
		case HTTP_STATUS_INFO:
			if (ch == ' ' || ch == '\t') {
				p->state = HTTP_WAIT_CR;
			} else if (ch == '\r') {
				p->state = HTTP_CR;
			}
			break;
		case HTTP_WAIT_CR:
			if (ch == '\r') {
				p->state = HTTP_CR;
			}
			break;
		case HTTP_CR:
			if (ch == '\n') {
				p->state = HTTP_CL;
			} else {
				ret = -1;
				goto end;
			}
			break;
		case HTTP_CL:
			if (ch == '\r') {
				p->state = HTTP_HDR_ALMOST_DONE;
			} else {
				wtk_strbuf_reset(buf);
				wtk_strbuf_push_c(buf, tolower(ch));
				p->state = HTTP_KEY;
			}
			break;
		case HTTP_KEY:
			if (ch == ':') {
				qtk_http_parser_save_key(p, buf->data, buf->pos);
				p->state = HTTP_KEY_SPACE;
			} else {
				wtk_strbuf_push_c(buf, tolower(ch));
			}
			break;
		case HTTP_KEY_SPACE:
			if (ch != ' ') {
				wtk_strbuf_reset(buf);
				wtk_strbuf_push_c(buf, ch);
				p->state = HTTP_VALUE;
			}
			break;
		case HTTP_VALUE:
			if (ch == '\r') {
				wtk_strbuf_strip(buf);
				qtk_httphdr_add(rep, p->key.data, p->key.len, buf->data, buf->pos);
				p->state = HTTP_CR;
			} else {
				wtk_strbuf_push_c(buf, ch);
			}
			break;
		case HTTP_HDR_ALMOST_DONE:
			if (ch == '\n') {
                wtk_strbuf_reset(buf);
                if (rep->content_length == 0) {
                    p->state = HTTP_CONTENT_DONE;
                    ret = 1;
                } else {
                    p->state = HTTP_HDR_DONE;
                    ret = qtk_parser_process_body(p, dq);
                }
                goto end;
			}
			break;
		default:
			wtk_debug("unexcepted state %d.\n", p->state);
			ret = -1;
			goto end;
		}
	}
end:
    return ret;
}


static int qtk_http_parser_feed(qtk_zhttp_parser_t *p, qtk_deque_t *dq) {
    char ch;
    int ret = 0;
    while (dq->len) {
        if (HTTP_START == p->state) {
            if (p->pack->content_length) {
                qtk_deque_pop(dq, NULL, p->pack->content_length);
                p->pack->content_length = 0;
            }
            qtk_deque_front(dq, &ch, 1);
            if (ch == ' ') {
                if (ngx_str4_cmp(p->tmp_buf->data, 'H', 'T', 'T', 'P')) {
                    p->type = HTTP_RESPONSE_PARSER;
                    p->state = HTTP_VERSION;
                } else {
                    p->type = HTTP_REQUEST_PARSER;
                    p->state = HTTP_METHOD;
                }
                qtk_httphdr_reset(p->pack);
            } else {
                wtk_strbuf_push_c(p->tmp_buf, ch);
                qtk_deque_pop(dq, &ch, 1);
            }
        } else {
            if (HTTP_HDR_DONE == p->state) {
                ret = qtk_parser_process_body(p, dq);
            } else if (HTTP_REQUEST_PARSER == p->type) {
                ret = qtk_http_parser_feed_request(p, dq);
            } else {
                ret = qtk_http_parser_feed_response(p, dq);
            }

            if (ret) {
                qtk_zhttp_parser_reset_state(p);
            }
            break;
        }
    }
    return ret;
}


qtk_zhttp_parser_t* qtk_zhttp_parser_new() {
    qtk_zhttp_parser_t *parser = wtk_malloc(sizeof(*parser));
    qtk_zhttp_parser_init(parser);
    return parser;
}


int qtk_zhttp_parser_delete(qtk_zhttp_parser_t *p) {
    wtk_strbuf_delete(p->tmp_buf);
	qtk_zhttp_parser_clean(p);
	wtk_free(p);
	return 0;
}


int qtk_zhttp_parser_init(qtk_zhttp_parser_t *p)
{
    p->tmp_buf = wtk_strbuf_new(1024, 1);
    p->handler=(qtk_parser_handler)qtk_http_parser_feed;
    p->get_body = (qtk_parser_getbody_handler)qtk_zhttp_parser_get_body;
	p->pack = qtk_httphdr_new();
    memset(p->handlers, 0, sizeof(p->handlers));
    qtk_zhttp_parser_reset_state(p);

	return 0;
}


int qtk_zhttp_parser_clean(qtk_zhttp_parser_t *p) {
    if (p->tmp_buf) {
        wtk_strbuf_delete(p->tmp_buf);
        p->tmp_buf = NULL;
    }
    if (p->pack) {
        qtk_httphdr_delete(p->pack);
        p->pack = NULL;
    }
    return 0;
}


int qtk_zhttp_parser_reset(qtk_zhttp_parser_t *p) {
    qtk_zhttp_parser_reset_state(p);
    if (p->tmp_buf) {
        wtk_strbuf_reset(p->tmp_buf);
    }
    if (p->pack) {
        qtk_httphdr_reset(p->pack);
    }
    return 0;
}


int qtk_zhttp_parser_reset_state(qtk_zhttp_parser_t *p) {
    p->state = HTTP_START;
    return 0;
}


int qtk_zhttp_parser_get_body(qtk_zhttp_parser_t *p, qtk_deque_t *dq,
                             qtk_deque_t *pack) {
    if (p->pack->content_length) {
        qtk_deque_move(pack, dq, p->pack->content_length);
        p->pack->content_length = 0;
    }
    return 0;
}
