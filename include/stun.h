#ifndef STUN_H
#define STUN_H

#include <stdint.h>

int32_t stun_handle(int n, char* msg, struct sockaddr* addr);

#endif // STUN_H
