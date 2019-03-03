#include "wtk/core/wtk_type.h"
#include "qtk_network.h"

#define DNS_CACHE 100

typedef struct qtk_dns_node qtk_dns_node_t;

struct qtk_dns_node {
    const char *hostname;
    struct in_addr addr;
    int update_time;
    int count;
};

/*
  cache not implement;
 */
//static qtk_dns_node_t dns_cache[DNS_CACHE+1] = {};

int qtk_get_ip_by_name(const char *hostname, struct in_addr* addr) {
    char buf[1024];
    struct hostent host_buf, *result = NULL;
    int err = 0;
    if (gethostbyname_r(hostname, &host_buf, buf, sizeof(buf), &result, &err)
        || !result || result->h_length <= 0) {
        wtk_debug("get ip by host failed: [%s]\r\n", hostname);
        return -1;
    }
    *addr = *(struct in_addr*)(result->h_addr);
    return 0;
}
