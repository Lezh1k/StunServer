#ifndef STUN_H
#define STUN_H

#include <stdint.h>

int32_t stun_prepare_message(int32_t n, char* msg, struct sockaddr* addr, int32_t* change_request);
int32_t stun_handle_change_addr(struct sockaddr* changed_addr, struct sockaddr *source_addr, void* pool);

#endif // STUN_H
