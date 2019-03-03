#include "qtk_http_parser.h"
#include "ctype.h"
#include "qtk/entry/qtk_socket.h"

#define HTTP_STATUS_MAP(no,s) \
	case no: return s;

const char *qtk_http_status_str(int status)
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


static int qtk_http_parser_claim_request(qtk_http_parser_t *p, qtk_socket_t *s) {
    if (!p->pack) {
        qtk_http_parser_pack_handler_t *handler = &p->handlers[HTTP_REQUEST_PARSER];
        if (handler->empty(handler->layer, p)) return -1;
        qtk_request_init((qtk_request_t*)p->pack, s);
    }
    return 0;
}


static int qtk_http_parser_claim_response(qtk_http_parser_t *p, qtk_socket_t *s) {
    if (!p->pack) {
        qtk_http_parser_pack_handler_t *handler = &p->handlers[HTTP_RESPONSE_PARSER];
        if (handler->empty(handler->layer, p)) return -1;
        qtk_response_init((qtk_response_t*)p->pack, s);
    }
    return 0;
}


static int qtk_http_parser_raise(qtk_http_parser_t *p) {
    if (p->pack) {
        qtk_http_parser_pack_handler_t *handler = &p->handlers[p->type];
        if (handler->ready) {
            handler->ready(handler->layer, p);
        }
        p->pack = NULL;
    }
    qtk_http_parser_reset_state(p);
    return 0;
}


static int qtk_http_parser_unlink(qtk_http_parser_t *p) {
    if (p->pack) {
        qtk_http_parser_pack_handler_t *handler = &p->handlers[p->type];
        if (p->state == HTTP_HDR_DONE || p->state == HTTP_CONTENT_DONE) {
            qtk_http_parser_raise(p);
        } else if (handler->unlink) {
            handler->unlink(handler->layer, p);
        }
        p->pack = NULL;
    }
    return 0;
}


static void qtk_http_parser_save_key(qtk_http_parser_t *p, wtk_heap_t *heap, char *data, int bytes) {
    p->key.data = wtk_heap_dup_data(heap, data, bytes);
    p->key.len = bytes;
}


static int qtk_http_parser_feed_request(qtk_http_parser_t *p, qtk_socket_t *sock, char *data, int len) {
	wtk_strbuf_t *buf = p->tmp_buf;
    qtk_request_t *r = p->pack;
	char ch;
	char *s, *e;
	int ret = 0;

    r->s = sock;
	s = data; e = s + len;
	while (s < e) {
		ch = *s;
		switch (p->state) {
		case HTTP_METHOD:
			if (ch == ' ') {
				if ((buf->pos==4) && ngx_str4_cmp(buf->data,'P','O','S','T')) {
					r->method=HTTP_POST;
				} else if((buf->pos==3) && ngx_str3_cmp(buf->data,'G','E','T')) {
					r->method=HTTP_GET;
				} else if((buf->pos==3) && ngx_str3_cmp(buf->data,'P','U','T')) {
					r->method=HTTP_PUT;
				} else if((buf->pos==4) && ngx_str4_cmp(buf->data,'H','E','A','D')) {
					r->method=HTTP_HEAD;
				} else if((buf->pos==6) && !memcmp(buf->data, "DELETE", 6)) {
					r->method=HTTP_DELETE;
				} else {
					wtk_debug("only POST,GET,PUT,DELETE,HEAD method is support now\r\n");
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
				qtk_request_set_string(r, &r->url, buf->data, buf->pos);
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
				qtk_request_set_string(r, &r->params, buf->data, buf->pos);
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
				qtk_http_parser_save_key(p, r->heap, buf->data, buf->pos);
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
				qtk_request_add_hdr(r, p->key.data, p->key.len,
                                    wtk_heap_dup_string(r->heap, buf->data, buf->pos));
				p->state = HTTP_CR;
			} else {
				wtk_strbuf_push_c(buf, ch);
			}
			break;
		case HTTP_HDR_ALMOST_DONE:
			if (ch == '\n') {
                qtk_request_update_hdr(r);
                wtk_strbuf_reset(buf);
                if (r->body) {
                    wtk_strbuf_reset(r->body);
                }
                if (r->content_length == 0) {
                    p->state = HTTP_CONTENT_DONE;
                    qtk_http_parser_raise(p);
                    s++;
                    goto end;
                } else {
                    p->state = HTTP_HDR_DONE;
                }
			}
			break;
        case HTTP_HDR_DONE:
        {
            int left, body_ret;
            body_ret = qtk_request_process_body(r, s, e - s, &left);
            s = e - left;
            if (body_ret > 0) {
                qtk_http_parser_raise(p);
            } else if (body_ret < 0) {
                ret = -1;
            }
            goto end;
        }
		default:
			wtk_debug("ah:never be here.\n");
			ret = -1;
			goto end;
		}
		++s;
	}
end:
	return ret == 0 ? e - s : ret;
}


static int qtk_http_parser_feed_response(qtk_http_parser_t *p, qtk_socket_t *sock, char *data, int len) {
    wtk_strbuf_t *buf = p->tmp_buf;
    qtk_response_t *rep = p->pack;
    wtk_string_t *v;
    char *s, *e;
    char ch;
    int ret = 0;

    rep->s = sock;
	s = data; e = data + len;
	while (s < e) {
		ch = *s;
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
				rep->status = wtk_str_atoi(buf->data, buf->pos);
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
				qtk_http_parser_save_key(p, rep->heap, buf->data, buf->pos);
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
				hash_str_node_t *node;
				wtk_strbuf_strip(buf);
				node = wtk_str_hash_find_node(rep->hash, p->key.data, p->key.len, 0);
				if (node) {
					v = (wtk_string_t*)node->value;
					wtk_strbuf_push_front_s(buf,";");
					wtk_strbuf_push_front(buf, v->data, v->len);
					wtk_heap_fill_string(rep->heap, v, buf->data, buf->pos);
				} else {
					v = wtk_heap_dup_string(rep->heap,buf->data,buf->pos);
					wtk_str_hash_add(rep->hash, p->key.data, p->key.len, v);
				}
				p->state = HTTP_CR;
			} else {
				wtk_strbuf_push_c(buf, ch);
			}
			break;
		case HTTP_HDR_ALMOST_DONE:
			if (ch == '\n') {
                qtk_response_update_hdr(rep);
                wtk_strbuf_reset(buf);
                if (rep->body) {
                    wtk_strbuf_reset(rep->body);
                }
                if (rep->content_length == 0) {
                    p->state = HTTP_CONTENT_DONE;
                    qtk_http_parser_raise(p);
                    s++;
                    goto end;
                } else {
                    p->state = HTTP_HDR_DONE;
                }
			}
			break;
        case HTTP_HDR_DONE:
        {
            int left, body_ret;
            body_ret = qtk_response_process_body(rep, s, e - s, &left);
            s = e - left;
            if (body_ret > 0) {
                qtk_http_parser_raise(p);
            } else if (body_ret < 0) {
                ret = -1;
            }
            goto end;
        }
		default:
			wtk_debug("unexcepted state %d.\n", p->state);
			ret = -1;
			goto end;
		}
		++s;
	}
end:
    return ret == 0 ? e -s : ret;
}


static int qtk_http_parser_feed(qtk_http_parser_t *p, qtk_socket_t *sock, char *buf, int len) {
    char *s = buf, *e = buf + len;
    while (s < e) {
        char ch = *s;
        if (HTTP_START == p->state) {
            if (ch == ' ') {
                if (ngx_str4_cmp(p->tmp_buf->data, 'H', 'T', 'T', 'P')) {
                    p->type = HTTP_RESPONSE_PARSER;
                    p->state = HTTP_VERSION;
                    qtk_http_parser_claim_response(p, sock);
                } else {
                    p->type = HTTP_REQUEST_PARSER;
                    p->state = HTTP_METHOD;
                    qtk_http_parser_claim_request(p, sock);
                }
            } else {
                wtk_strbuf_push_c(p->tmp_buf, ch);
                s++;
            }
        } else {
            int left;

            if (HTTP_REQUEST_PARSER == p->type) {
                left = qtk_http_parser_feed_request(p, sock, s, e - s);
            } else {
                left = qtk_http_parser_feed_response(p, sock, s, e - s);
            }

            if (left < 0) {
                qtk_http_parser_reset_state(p);
                s = e;
            } else {
                s = e - left;
            }
        }
    }
    return 0;
}


static int qtk_http_parser_close(qtk_http_parser_t *p) {
    wtk_strbuf_delete(p->tmp_buf);
	qtk_http_parser_reset(p);
    return 0;
}


qtk_http_parser_t* qtk_http_parser_new() {
	qtk_http_parser_t* p;

	p=(qtk_http_parser_t*)wtk_calloc(1,sizeof(*p));
    p->tmp_buf = wtk_strbuf_new(1024, 1);
    p->reset_handler = (qtk_parser_reset_handler)qtk_http_parser_reset_state;
    p->close_handler = (qtk_parser_close_handler)qtk_http_parser_close;
    qtk_http_parser_init(p);

	return p;
}

qtk_http_parser_t* qtk_http_parser_clone(qtk_http_parser_t* p) {
	qtk_http_parser_t* parser;

	parser = (qtk_http_parser_t*)wtk_calloc(1,sizeof(*parser));
    *parser = *p;
    parser->tmp_buf = wtk_strbuf_new(1024, 1);
    parser->pack = NULL;
    qtk_http_parser_reset_state(parser);

    return parser;
}


int qtk_http_parser_delete(qtk_http_parser_t *p)
{
    qtk_http_parser_close(p);
	wtk_free(p);
	return 0;
}

int qtk_http_parser_init(qtk_http_parser_t *p)
{
    p->handler=(qtk_parser_handler)qtk_http_parser_feed;
	p->pack = NULL;
    memset(p->handlers, 0, sizeof(p->handlers));

	return 0;
}

int qtk_http_parser_bytes(qtk_http_parser_t *p)
{
	return sizeof(*p);
}

int qtk_http_parser_reset(qtk_http_parser_t *p)
{
    qtk_http_parser_unlink(p);
    qtk_http_parser_init(p);

	return 0;
}

int qtk_http_parser_reset_state(qtk_http_parser_t *p)
{
    qtk_http_parser_unlink(p);
    p->state = HTTP_START;

    return 0;
}


void qtk_http_parser_set_handlers(qtk_http_parser_t *p, parser_type_t type,
                                  qtk_http_parser_pack_f empty,
                                  qtk_http_parser_pack_f ready,
                                  qtk_http_parser_pack_f unlink,
                                  void *layer) {
    p->handlers[type].empty = empty;
    p->handlers[type].ready = ready;
    p->handlers[type].unlink = unlink;
    p->handlers[type].layer = layer;
}
