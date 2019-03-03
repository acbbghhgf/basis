#ifndef QTK_CORE_QTK_NETWORK_H
#define QTK_CORE_QTK_NETWORK_H
#include <netdb.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

int qtk_get_ip_by_name(const char *hostname, struct in_addr *addr);

#ifdef __cplusplus
}
#endif

#endif
