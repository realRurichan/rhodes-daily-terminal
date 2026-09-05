#ifndef DAILY_TERMINAL_PLATFORM_H
#define DAILY_TERMINAL_PLATFORM_H

#include "app.h"

typedef struct {
  int input_fd;
  int interactive;
} DtPlatform;

int dt_platform_init(DtPlatform *platform);
void dt_platform_shutdown(DtPlatform *platform);
DtKey dt_platform_read_key(DtPlatform *platform, int timeout_ms);
void dt_platform_draw(const char *frame);
const char *dt_platform_data_path(void);

#endif

