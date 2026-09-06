#ifndef DAILY_TERMINAL_APP_H
#define DAILY_TERMINAL_APP_H

#include <stddef.h>
#include <time.h>

#define DT_TASK_COUNT 3
#define DT_TEXT_SIZE 96

typedef enum {
  DT_SCREEN_HOME,
  DT_SCREEN_FOCUS,
  DT_SCREEN_TASKS,
  DT_SCREEN_EVENT,
  DT_SCREEN_PET,
  DT_SCREEN_STATUS,
  DT_SCREEN_LANGUAGE,
  DT_SCREEN_EXIT
} DtScreen;

typedef enum { DT_KEY_NONE, DT_KEY_UP, DT_KEY_DOWN, DT_KEY_OK, DT_KEY_BACK } DtKey;

typedef struct {
  char title[DT_TEXT_SIZE];
  int completed;
} DtTask;

typedef struct {
  int level;
  int xp;
  int coins;
  int streak;
  int pet_mood;
  int pet_hunger;
  int pet_energy;
  int pet_bond;
  int pet_xp;
  int focus_minutes;
  int focus_running;
  time_t focus_end;
  /* Kept at 11 bytes so saves made by app_ver 3 remain binary-compatible. */
  char language[11];
  DtTask tasks[DT_TASK_COUNT];
} DtSave;

typedef struct {
  DtScreen screen;
  int cursor;
  int quit;
  unsigned int rng;
  char notice[DT_TEXT_SIZE];
  DtSave save;
} DtApp;

void dt_app_init(DtApp *app, time_t now);
void dt_app_handle(DtApp *app, DtKey key, time_t now);
void dt_app_tick(DtApp *app, time_t now);
void dt_app_render(const DtApp *app, char *out, size_t capacity, time_t now);
void dt_app_validate_settings(DtApp *app);

#endif
