#include "app.h"
#include "platform.h"
#include "storage.h"

#include <stdio.h>
#include <time.h>

int main(void) {
  DtPlatform platform;
  DtApp app;
  char frame[4096];
  time_t now = time(NULL);
  dt_app_init(&app, now);
  dt_save_load(dt_platform_data_path(), &app.save);
  dt_app_validate_settings(&app);
  if (!dt_platform_init(&platform)) return 1;
  while (!app.quit) {
    now = time(NULL);
    dt_app_tick(&app, now);
    dt_app_render(&app, frame, sizeof(frame), now);
    dt_platform_draw(frame);
    DtKey key = dt_platform_read_key(&platform, 250);
    if (key != DT_KEY_NONE) {
      dt_app_handle(&app, key, time(NULL));
      dt_save_write(dt_platform_data_path(), &app.save);
    }
  }
  dt_save_write(dt_platform_data_path(), &app.save);
  dt_platform_shutdown(&platform);
  puts("\nPRTS session closed.");
  return 0;
}
