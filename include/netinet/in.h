#ifndef __C1_NETINET_IN_H
#define __C1_NETINET_IN_H
#include <stdint.h>
#include <sys/socket.h>
typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
struct in_addr {
    in_addr_t s_addr;
};
struct sockaddr_in {
    unsigned short sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;
    unsigned char sin_zero[8];
};
#define INADDR_ANY 0
#define INADDR_LOOPBACK 0x7f000001
uint16_t htons(uint16_t value);
uint16_t ntohs(uint16_t value);
uint32_t htonl(uint32_t value);
uint32_t ntohl(uint32_t value);
#endif
