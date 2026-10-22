#ifndef __C1_NETDB_H
#define __C1_NETDB_H
#include <sys/socket.h>
#include <sys/types.h>
struct addrinfo {
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
    socklen_t ai_addrlen;
    struct sockaddr *ai_addr;
    char *ai_canonname;
    struct addrinfo *ai_next;
};
#define AI_PASSIVE 1
#define AI_CANONNAME 2
#define AI_NUMERICHOST 4
#define NI_NUMERICHOST 1
#define NI_NUMERICSERV 2
int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints,
                struct addrinfo **result);
void freeaddrinfo(struct addrinfo *result);
const char *gai_strerror(int error);
int getnameinfo(const struct sockaddr *address, socklen_t length, char *host,
                socklen_t host_length, char *service, socklen_t service_length, int flags);
#endif
