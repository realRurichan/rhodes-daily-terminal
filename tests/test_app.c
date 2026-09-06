#include "app.h"
#include "storage.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  time_t now = 1704067200;
  DtApp app;
  dt_app_init(&app, now);
  assert(app.save.level == 1);
  dt_app_handle(&app, DT_KEY_OK, now);
  assert(app.screen == DT_SCREEN_FOCUS);
  dt_app_handle(&app, DT_KEY_OK, now);
  assert(app.save.focus_running);
  dt_app_tick(&app, app.save.focus_end);
  assert(!app.save.focus_running);
  assert(app.save.tasks[0].completed);
  assert(app.save.coins == 1200);

  app.screen = DT_SCREEN_PET;
  app.cursor = 0;
  dt_app_handle(&app, DT_KEY_OK, now);
  assert(app.save.coins == 1100);
  assert(app.save.pet_hunger == 100);
  app.screen = DT_SCREEN_EXIT;
  dt_app_handle(&app, DT_KEY_OK, now);
  assert(app.quit);

  const char *path = "daily_terminal_test.sav";
  assert(dt_save_write(path, &app.save));
  DtSave loaded = {0};
  assert(dt_save_load(path, &loaded));
  assert(loaded.coins == app.save.coins);
  remove(path);
  FILE *bad = fopen(path, "wb");
  assert(bad);
  fputs("truncated", bad);
  fclose(bad);
  memset(&loaded, 0, sizeof(loaded));
  assert(!dt_save_load(path, &loaded));
  remove(path);
  puts("all tests passed");
  return 0;
}
