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
  assert(strcmp(app.save.language, "zh") == 0);
  char frame[4096];
  dt_app_render(&app, frame, sizeof(frame), now);
  assert(strstr(frame, "2024-") == NULL);
  assert(strstr(frame, "KEY1/2") != NULL);
  assert(strstr(frame, "专注行动") != NULL);
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

  app.screen = DT_SCREEN_EVENT;
  app.rng = 0;
  unsigned int expected_rng = 0;
  int expected_coins = app.save.coins;
  for (int i = 0; i < 20; ++i) {
    expected_rng = expected_rng * 1103515245u + 12345u;
    unsigned int event = expected_rng % 5u;
    expected_coins += event == 2u ? 37 : (event == 4u ? 120 : 0);
    dt_app_handle(&app, DT_KEY_OK, now);
    assert(app.save.coins == expected_coins);
  }

  app.screen = DT_SCREEN_PET;
  app.cursor = 1;
  int bond_before = app.save.pet_bond;
  dt_app_handle(&app, DT_KEY_OK, now);
  assert(app.save.pet_bond == bond_before + 5);
  assert(app.save.pet_xp == 3);
  app.cursor = 2;
  dt_app_handle(&app, DT_KEY_OK, now);
  assert(app.save.pet_energy <= 100);
  app.cursor = 3;
  int coins_before_explore = app.save.coins;
  dt_app_handle(&app, DT_KEY_OK, now);
  assert(app.save.coins >= coins_before_explore + 50);
  assert(app.save.coins <= coins_before_explore + 300);
  assert(app.save.pet_xp == 11);

  app.screen = DT_SCREEN_LANGUAGE;
  dt_app_handle(&app, DT_KEY_DOWN, now);
  assert(strcmp(app.save.language, "en") == 0);
  dt_app_render(&app, frame, sizeof(frame), now);
  assert(strstr(frame, "ENGLISH") != NULL);
  dt_app_handle(&app, DT_KEY_OK, now);
  assert(app.screen == DT_SCREEN_HOME);

  app.screen = DT_SCREEN_EXIT;
  dt_app_handle(&app, DT_KEY_BACK, now);
  assert(!app.quit);
  assert(app.screen == DT_SCREEN_HOME);
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
