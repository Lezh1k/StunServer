#ifndef LOGGER_H
#define LOGGER_H

#include "commons.h"

typedef enum log_level {
  LL_EMERG    = 0, /* system is unusable */
  LL_ALERT    = 1, /* action must be taken immediately */
  LL_CRIT     = 2, /* critical conditions */
  LL_ERR      = 3, /* error conditions */
  LL_WARNING  = 4, /* warning conditions */
  LL_NOTICE   =	5, /* normal but significant condition */
  LL_INFO     =	6, /* informational */
  LL_DEBUG    = 7 /* debug-level messages */
} log_level_t;

void logger_init();
void logger_shutdown();

void logger_log(log_level_t level, const char* format, ...);

#endif // LOGGER_H
