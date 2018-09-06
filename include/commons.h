#ifndef COMMONS_H
#define COMMONS_H
#include <stdint.h>

#define UNUSED_ARG(x) ((void)x)
#define PROGRAM_NAME  "mad stun 1.0.1\0"

typedef struct settings {
  char addr0[16];
  uint16_t port0;
  char addr1[16];
  uint16_t port1;
} settings_t ;


#endif // COMMONS_H
