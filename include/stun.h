#ifndef STUN_H
#define STUN_H

#include <stdint.h>

int32_t stun_prepare_message(int32_t n,
                             char* restrict msg,
                             struct sockaddr* restrict addr,
                             int32_t* restrict change_request);
int32_t stun_handle_change_addr(struct sockaddr * restrict changed_addr,
                                struct sockaddr * restrict source_addr,
                                void * restrict  pool);

#endif // STUN_H
