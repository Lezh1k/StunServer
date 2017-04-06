#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>
#include "logger.h"

const char* prefixes[] = {
  "EMERG    ", // 0, /* system is unusable */
  "ALERT    ", // 1, /* action must be taken immediately */
  "CRIT     ", // 2, /* critical conditions */
  "ERR      ", // 3, /* error conditions */
  "WARNING  ", // 4, /* warning conditions */
  "NOTICE   ", //	5, /* normal but significant condition */
  "INFO     ", //	6, /* informational */
  "DEBUG    ", // 7  /* debug-level messages */
};

pthread_mutex_t lock;

void
logger_init() {
  openlog(PROGRAM_NAME, 0, LOG_USER);
  pthread_mutex_init(&lock, NULL);
  logger_log(LL_NOTICE, "Logger for %s is initialized", PROGRAM_NAME);
}
//////////////////////////////////////////////////////////////

void
logger_shutdown() {
  pthread_mutex_destroy(&lock);
  closelog();
}
//////////////////////////////////////////////////////////////

char message_buffer[1024] = {0};
void
logger_log(log_level_t level,
           const char *format, ...) {
  va_list args;
  int n;
  pthread_mutex_lock(&lock);
  va_start(args, format);
  n = vsprintf(message_buffer, format, args);
  message_buffer[n] = 0;
  printf(prefixes[level]);
  printf("%s\n", message_buffer);
  syslog(level, "%s", message_buffer);
  va_end(args);
  pthread_mutex_unlock(&lock);
}
//////////////////////////////////////////////////////////////
