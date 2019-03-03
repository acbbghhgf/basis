#ifndef _MAIN_QTK_DLG_RES_H_
#define _MAIN_QTK_DLG_RES_H_

typedef struct qtk_dlg_res qtk_dlg_res_t;
typedef enum {
    QTK_DLG_RES_BUILTIN,
    QTK_DLG_RES_TMP,
} qtk_dlg_res_type;

struct qtk_dlg_res {
    qtk_dlg_res_type type;
    char *location;         // must valid after qtk_dlg_res_parse return
};

int qtk_dlg_res_parse(const char *res, qtk_dlg_res_t *r);

#endif
