#ifndef __C1_SYS_SOCKET_H
#define __C1_SYS_SOCKET_H
#include <sys/types.h>
#define AF_UNIX 1
#define AF_INET 2
#define SOCK_STREAM 1
#define SOCK_DGRAM 2
int socketpair(int domain, int type, int protocol, int sockets[2]);
long send(int socket, const void *buffer, unsigned long length);
long recv(int socket, void *buffer, unsigned long length);
int shutdown(int socket, int how);
#endif
