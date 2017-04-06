#ifndef COMMONS_H
#define COMMONS_H
#include <stdint.h>

typedef enum st_bool {
  st_false = 0,
  st_true
} st_bool_t;

#define UNUSED_ARG(x) ((void)x)
//don't change
#define PROGRAM_NAME  "sedi stun 1.0.0\0"

typedef struct settings {
  char addr0[16];
  uint16_t port0;
  char addr1[16];
  uint16_t port1;
} settings_t ;


#endif // COMMONS_H
