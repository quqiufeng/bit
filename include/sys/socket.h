#ifndef __C1_SYS_SOCKET_H
#define __C1_SYS_SOCKET_H
#include <sys/types.h>
typedef unsigned int socklen_t;
struct sockaddr {
    unsigned short sa_family;
    char sa_data[14];
};
#define AF_UNSPEC 0
#define AF_UNIX 1
#define AF_INET 2
#define SOCK_STREAM 1
#define SOCK_DGRAM 2
int socketpair(int domain, int type, int protocol, int sockets[2]);
int shutdown(int socket, int how);
int socket(int domain, int type, int protocol);
int bind(int socket, const struct sockaddr *address, socklen_t length);
int listen(int socket, int backlog);
int accept(int socket, struct sockaddr *address, socklen_t *length);
int connect(int socket, const struct sockaddr *address, socklen_t length);
int getsockname(int socket, struct sockaddr *address, socklen_t *length);
long send(int socket, const void *buffer, unsigned long length, int flags);
long recv(int socket, void *buffer, unsigned long length, int flags);
#endif
