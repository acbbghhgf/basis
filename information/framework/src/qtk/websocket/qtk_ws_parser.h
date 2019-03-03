#ifndef _QTK_WEBSOCKET_QTK_WS_PARSER_H
#define _QTK_WEBSOCKET_QTK_WS_PARSER_H

#ifdef __cplusplus
#extern "C" {
#endif

#ifdef __cplusplus
}
#endif

struct qtk_ws_parser {
    QTK_PARSER
    wtk_queue_node_t n;
    wtk_strbuf_t *tmpbuf;
    qtk_ws_buffer_t *pack;
    qtk_ws_parser_state_t state;
    qtk_ws_parser_pack_handler_t pack_handler;
};

qtk_ws_parser_t* qtk_ws_parser_new();
int qtk_ws_parser_delete(qtk_ws_parser_t *p);
int qtk_ws_parser_init(qtk_ws_parser_t *p);
qtk_ws_parser_t* qtk_ws_parser_clone(qtk_ws_parser_t *p);
int qtk_ws_parser_reset(qtk_ws_parser_t *p);
int qtk_ws_parser_reset_state(qtk_ws_parser_t *p);
void qtk_ws_parser_set_handlers(qtk_ws_parser_t *p,
                                qtk_ws_parser_pack_f empty,
                                qtk_ws_parser_pack_f ready,
                                qtk_ws_parser_pack_f unlink,
                                void *layer);


#endif
