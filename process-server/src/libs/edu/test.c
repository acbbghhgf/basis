#include "qtk_eval_module.c"


int main() {
    eval_cfg_t *ecfg = wtk_malloc(sizeof(*ecfg));
    ecfg->cfg = wtk_engsnt_cfg_new_bin3("../../../ext/eng.eval.snt.bin", 0);
    ecfg->dst_sample_rate = 16000;
    void *inst = eval_create(ecfg);
    wtk_string_t param = wtk_string("{\"text\":\"tee\"}");
    eval_start(inst, &param);
    int fd = open("../../../data/tee.wav", O_RDONLY);
    char buf[1024];
    int n;
    while ((n = read(fd, buf, sizeof(buf)))) {
        wtk_string_t wav;
        wtk_string_set(&wav, buf, n);
        eval_feed(inst, &wav);
    }
    wtk_string_t rst;
    wtk_string_set(&rst, NULL, 0);
    eval_stop(inst);
    eval_get_result(inst, &rst);
    printf("%.*s\r\n", rst.len, rst.data);
    eval_release(inst);
    eval_clean(ecfg);
    return 0;
}
