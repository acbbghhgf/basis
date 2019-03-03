#include <assert.h>
#include "qtk/util/qtk_deque.h"


static int _str_cmp(const char* buf, const char *sub, int n) {
    int i;
    int ret = 0;
    int len = strlen(sub);
    for (i = 0; i < n; ++i) {
        if ((ret = memcmp(buf, sub, len))) break;
        buf += len;
    }
    return ret;
}


void test_deque_tail() {
    qtk_deque_t *dq = qtk_deque_new(1024, 1024*1024, 1);
    int i;
    char *buf = wtk_malloc(10000);
    for (i = 0; i < 1000; ++i) {
        qtk_deque_push(dq, "helloworld", 10);
    }

    qtk_deque_tail(dq, buf, 10);
    assert(0 == _str_cmp(buf, "helloworld", 1));
    for (i = 10; i <= 1000; i+=10) {
        qtk_deque_tail(dq, buf, i*10);
        assert(0 == _str_cmp(buf, "helloworld", i));
    }
    wtk_free(buf);
    qtk_deque_delete(dq);
}
