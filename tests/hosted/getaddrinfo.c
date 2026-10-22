#include <netdb.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    struct addrinfo hints;
    struct addrinfo *result;
    int status;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    status = getaddrinfo("localhost", "80", &hints, &result);
    if (status != 0)
        return 1;
    if (!result || !result->ai_addr)
        return 2;
    freeaddrinfo(result);
    puts("hosted getaddrinfo ok");
    return 0;
}
