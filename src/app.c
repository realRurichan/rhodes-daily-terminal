#include "app.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static const char *menu[] = {"FOCUS OPERATION", "DAILY MISSIONS", "FIELD EVENT", "PET TERMINAL", "OPERATOR STATUS", "EXIT PROGRAM"};
static const char *events[] = {
    "Amiya sent an encouraging message.",
    "A lost Originium Slug follows you.",
    "Closure found 37 LMD under a console.",
    "Weather clear. Rhodes Island proceeds.",
    "PRTS reports: probability of success 87%."};

static unsigned int next_random(DtApp *app) {
  app->rng = app->rng * 1103515245u + 12345u;
  return app->rng;
}

static int completed_tasks(const DtApp *app) {
  int n = 0;
  for (int i = 0; i < DT_TASK_COUNT; ++i) n += app->save.tasks[i].completed != 0;
  return n;
}

void dt_app_set_day(DtApp *app, time_t now) {
  struct tm *tmv = localtime(&now);
  char today[11] = "1970-01-01";
  if (tmv) strftime(today, sizeof(today), "%Y-%m-%d", tmv);
  if (strcmp(app->save.day, today) != 0) {
    snprintf(app->save.day, sizeof(app->save.day), "%s", today);
    snprintf(app->save.tasks[0].title, DT_TEXT_SIZE, "Complete one focus operation");
    snprintf(app->save.tasks[1].title, DT_TEXT_SIZE, "Review today's priorities");
    snprintf(app->save.tasks[2].title, DT_TEXT_SIZE, "Take a proper break");
    for (int i = 0; i < DT_TASK_COUNT; ++i) app->save.tasks[i].completed = 0;
    app->save.streak += 1;
  }
}

void dt_app_init(DtApp *app, time_t now) {
  memset(app, 0, sizeof(*app));
  app->save.level = 1;
  app->save.pet_mood = 70;
  app->save.pet_hunger = 75;
  app->save.pet_energy = 80;
  app->save.focus_minutes = 25;
  app->rng = (unsigned int)now;
  dt_app_set_day(app, now);
}

void dt_app_tick(DtApp *app, time_t now) {
  if (app->save.focus_running && now >= app->save.focus_end) {
    app->save.focus_running = 0;
    app->save.xp += 25;
    app->save.coins += 1200;
    app->save.pet_mood += 8;
    if (app->save.pet_mood > 100) app->save.pet_mood = 100;
    app->save.tasks[0].completed = 1;
    snprintf(app->notice, sizeof(app->notice), "Operation complete: +25 XP, +1200 LMD");
    while (app->save.xp >= app->save.level * 100) {
      app->save.xp -= app->save.level * 100;
      app->save.level++;
    }
  }
}

void dt_app_handle(DtApp *app, DtKey key, time_t now) {
  dt_app_tick(app, now);
  if (key == DT_KEY_NONE) return;
  app->notice[0] = '\0';
  if (key == DT_KEY_BACK) {
    if (app->screen == DT_SCREEN_HOME) app->quit = 1;
    else { app->screen = DT_SCREEN_HOME; app->cursor = 0; }
    return;
  }
  if (app->screen == DT_SCREEN_HOME) {
    if (key == DT_KEY_UP) app->cursor = (app->cursor + 5) % 6;
    if (key == DT_KEY_DOWN) app->cursor = (app->cursor + 1) % 6;
    if (key == DT_KEY_OK) { app->screen = (DtScreen)(DT_SCREEN_FOCUS + app->cursor); app->cursor = 0; }
  } else if (app->screen == DT_SCREEN_FOCUS) {
    if (!app->save.focus_running && key == DT_KEY_UP && app->save.focus_minutes < 60) app->save.focus_minutes += 5;
    if (!app->save.focus_running && key == DT_KEY_DOWN && app->save.focus_minutes > 5) app->save.focus_minutes -= 5;
    if (key == DT_KEY_OK) {
      app->save.focus_running = !app->save.focus_running;
      if (app->save.focus_running) app->save.focus_end = now + app->save.focus_minutes * 60;
      snprintf(app->notice, sizeof(app->notice), app->save.focus_running ? "Operation started." : "Operation aborted.");
    }
  } else if (app->screen == DT_SCREEN_TASKS) {
    if (key == DT_KEY_UP) app->cursor = (app->cursor + DT_TASK_COUNT - 1) % DT_TASK_COUNT;
    if (key == DT_KEY_DOWN) app->cursor = (app->cursor + 1) % DT_TASK_COUNT;
    if (key == DT_KEY_OK) app->save.tasks[app->cursor].completed = !app->save.tasks[app->cursor].completed;
  } else if (app->screen == DT_SCREEN_EVENT && key == DT_KEY_OK) {
    unsigned int index = next_random(app) % (sizeof(events) / sizeof(events[0]));
    snprintf(app->notice, sizeof(app->notice), "%s", events[index]);
    int reward = index == 2 ? 37 : (index == 4 ? 120 : 0);
    app->save.coins += reward;
    app->save.pet_mood += index == 0 ? 6 : 2;
    if (app->save.pet_mood > 100) app->save.pet_mood = 100;
    if (reward) snprintf(app->notice, sizeof(app->notice), "Event reward: +%d LMD", reward);
  } else if (app->screen == DT_SCREEN_PET) {
    if (key == DT_KEY_UP) app->cursor = (app->cursor + 3) % 4;
    if (key == DT_KEY_DOWN) app->cursor = (app->cursor + 1) % 4;
    if (key == DT_KEY_OK && app->cursor == 0) {
      if (app->save.coins < 100) snprintf(app->notice, sizeof(app->notice), "Not enough LMD for food.");
      else { app->save.coins -= 100; app->save.pet_hunger += 28; app->save.pet_mood += 3; snprintf(app->notice, sizeof(app->notice), "Pet enjoyed the meal."); }
    } else if (key == DT_KEY_OK && app->cursor == 1) {
      if (app->save.pet_energy < 12) snprintf(app->notice, sizeof(app->notice), "Pet needs to rest first.");
      else { app->save.pet_energy -= 12; app->save.pet_mood += 15; app->save.pet_bond += 5; app->save.pet_xp += 3; snprintf(app->notice, sizeof(app->notice), "Bond increased."); }
    } else if (key == DT_KEY_OK && app->cursor == 2) {
      app->save.pet_energy += 30; app->save.pet_hunger -= 5; snprintf(app->notice, sizeof(app->notice), "Pet is sleeping.");
    } else if (key == DT_KEY_OK && app->cursor == 3) {
      if (app->save.pet_energy < 25 || app->save.pet_hunger < 20) snprintf(app->notice, sizeof(app->notice), "Pet cannot explore now.");
      else { int gain = 50 + (int)(next_random(app) % 251); app->save.pet_energy -= 25; app->save.pet_hunger -= 12; app->save.coins += gain; app->save.pet_xp += 8; snprintf(app->notice, sizeof(app->notice), "Exploration: +%d LMD", gain); }
    }
    if (app->save.pet_mood > 100) app->save.pet_mood = 100;
    if (app->save.pet_hunger > 100) app->save.pet_hunger = 100;
    if (app->save.pet_energy > 100) app->save.pet_energy = 100;
    if (app->save.pet_bond > 100) app->save.pet_bond = 100;
  } else if (app->screen == DT_SCREEN_EXIT && key == DT_KEY_OK) {
    app->quit = 1;
  }
}

static void append(char *out, size_t cap, size_t *used, const char *fmt, ...) {
  if (*used >= cap) return;
  va_list args;
  va_start(args, fmt);
  int n = vsnprintf(out + *used, cap - *used, fmt, args);
  va_end(args);
  if (n > 0) *used += (size_t)n < cap - *used ? (size_t)n : cap - *used;
}

void dt_app_render(const DtApp *app, char *out, size_t capacity, time_t now) {
  size_t used = 0;
  if (!capacity) return;
  out[0] = '\0';
  append(out, capacity, &used, "========================================\n RHODES ISLAND // DAILY TERMINAL\n %s  LV.%02d  LMD %06d\n========================================\n", app->save.day, app->save.level, app->save.coins);
  if (app->screen == DT_SCREEN_HOME) {
    append(out, capacity, &used, "\n PRTS ONLINE     Missions %d/%d\n Pet mood: %d%%\n\n", completed_tasks(app), DT_TASK_COUNT, app->save.pet_mood);
    for (int i = 0; i < 6; ++i) append(out, capacity, &used, " %c %s\n", i == app->cursor ? '>' : ' ', menu[i]);
  } else if (app->screen == DT_SCREEN_FOCUS) {
    long remain = app->save.focus_running ? (long)(app->save.focus_end - now) : app->save.focus_minutes * 60L;
    if (remain < 0) remain = 0;
    append(out, capacity, &used, "\n FOCUS OPERATION\n\n          %02ld:%02ld\n\n %s\n", remain / 60, remain % 60, app->save.focus_running ? "[OK] ABORT" : "[UP/DOWN] TIME  [OK] START");
  } else if (app->screen == DT_SCREEN_TASKS) {
    append(out, capacity, &used, "\n DAILY MISSIONS\n\n");
    for (int i = 0; i < DT_TASK_COUNT; ++i) append(out, capacity, &used, " %c [%c] %s\n", i == app->cursor ? '>' : ' ', app->save.tasks[i].completed ? 'x' : ' ', app->save.tasks[i].title);
  } else if (app->screen == DT_SCREEN_EVENT) {
    append(out, capacity, &used, "\n FIELD EVENT\n\n Press [OK] to scan local sector.\n");
  } else if (app->screen == DT_SCREEN_PET) {
    static const char *actions[] = {"FEED - 100 LMD", "PLAY", "REST", "EXPLORE"};
    const char *stage = app->save.pet_xp >= 120 ? "ELITE ORIGINIUM BEAST" : app->save.pet_xp >= 40 ? "ACTIVE ORIGINIUM SLUG" : "YOUNG ORIGINIUM SLUG";
    append(out, capacity, &used, "\n PET TERMINAL\n %s\n Mood %d  Hunger %d\n Energy %d  Bond %d\n\n", stage, app->save.pet_mood, app->save.pet_hunger, app->save.pet_energy, app->save.pet_bond);
    for (int i = 0; i < 4; ++i) append(out, capacity, &used, " %c %s\n", i == app->cursor ? '>' : ' ', actions[i]);
  } else if (app->screen == DT_SCREEN_STATUS) {
    append(out, capacity, &used, "\n OPERATOR STATUS\n\n Level: %d\n XP: %d/%d\n Streak: %d days\n Pet mood: %d%%\n Missions: %d/%d\n", app->save.level, app->save.xp, app->save.level * 100, app->save.streak, app->save.pet_mood, completed_tasks(app), DT_TASK_COUNT);
  } else append(out, capacity, &used, "\n EXIT PROGRAM\n\n End this terminal session?\n [OK] SAVE AND EXIT\n [BACK] CANCEL\n");
  if (app->notice[0]) append(out, capacity, &used, "\n PRTS> %s\n", app->notice);
  append(out, capacity, &used, "\n [W/S] Move  [Enter] Confirm  [Q] Back\n");
}
