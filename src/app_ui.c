#include "app.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define MENU_COUNT 7

static const char *menu_en[] = {"FOCUS", "MISSIONS", "FIELD EVENT", "PET", "STATUS", "LANGUAGE", "EXIT"};
static const char *menu_zh[] = {"专注行动", "每日任务", "区域事件", "电子宠物", "干员状态", "语言设置", "退出程序"};
static const char *task_en[] = {"Complete one focus operation", "Review today's priorities", "Take a proper break"};
static const char *task_zh[] = {"完成一次专注行动", "确认今天的优先事项", "进行一次充分休息"};
static const char *event_en[] = {"Amiya sent encouragement.", "A lost slug follows you.", "Closure found 37 LMD.", "Weather clear. Moving on.", "PRTS success forecast: 87%."};
static const char *event_zh[] = {"阿米娅发来了鼓励。", "一只迷路的源石虫跟着你。", "可露希尔找到了37龙门币。", "天气晴朗，罗德岛继续前进。", "PRTS预测行动成功率为87%。"};

static int zh(const DtApp *app) { return strcmp(app->save.language, "zh") == 0; }
static unsigned int next_random(DtApp *app) { app->rng = app->rng * 1103515245u + 12345u; return app->rng; }
static int completed(const DtApp *app) {
  int count = 0;
  for (int i = 0; i < DT_TASK_COUNT; ++i) count += app->save.tasks[i].completed != 0;
  return count;
}

void dt_app_validate_settings(DtApp *app) {
  if (strcmp(app->save.language, "en") != 0 && strcmp(app->save.language, "zh") != 0)
    snprintf(app->save.language, sizeof(app->save.language), "zh");
}

void dt_app_init(DtApp *app, time_t now) {
  memset(app, 0, sizeof(*app));
  app->save.level = 1;
  app->save.pet_mood = 70;
  app->save.pet_hunger = 75;
  app->save.pet_energy = 80;
  app->save.focus_minutes = 25;
  snprintf(app->save.language, sizeof(app->save.language), "zh");
  for (int i = 0; i < DT_TASK_COUNT; ++i) snprintf(app->save.tasks[i].title, DT_TEXT_SIZE, "%s", task_en[i]);
  app->rng = (unsigned int)now;
}

void dt_app_tick(DtApp *app, time_t now) {
  if (!app->save.focus_running || now < app->save.focus_end) return;
  app->save.focus_running = 0;
  app->save.xp += 25;
  app->save.coins += 1200;
  app->save.pet_mood += 8;
  if (app->save.pet_mood > 100) app->save.pet_mood = 100;
  app->save.tasks[0].completed = 1;
  snprintf(app->notice, sizeof(app->notice), "%s", zh(app) ? "行动完成：+25经验，+1200龙门币" : "Complete: +25 XP, +1200 LMD");
  while (app->save.xp >= app->save.level * 100) { app->save.xp -= app->save.level * 100; app->save.level++; }
}

void dt_app_handle(DtApp *app, DtKey key, time_t now) {
  dt_app_tick(app, now);
  if (key == DT_KEY_NONE) return;
  app->notice[0] = '\0';
  if (key == DT_KEY_BACK) {
    if (app->screen == DT_SCREEN_HOME) app->screen = DT_SCREEN_EXIT;
    else { app->screen = DT_SCREEN_HOME; app->cursor = 0; }
    return;
  }
  if (app->screen == DT_SCREEN_HOME) {
    if (key == DT_KEY_UP) app->cursor = (app->cursor + MENU_COUNT - 1) % MENU_COUNT;
    if (key == DT_KEY_DOWN) app->cursor = (app->cursor + 1) % MENU_COUNT;
    if (key == DT_KEY_OK) { app->screen = (DtScreen)(DT_SCREEN_FOCUS + app->cursor); app->cursor = 0; }
  } else if (app->screen == DT_SCREEN_FOCUS) {
    if (!app->save.focus_running && key == DT_KEY_UP && app->save.focus_minutes < 60) app->save.focus_minutes += 5;
    if (!app->save.focus_running && key == DT_KEY_DOWN && app->save.focus_minutes > 5) app->save.focus_minutes -= 5;
    if (key == DT_KEY_OK) {
      app->save.focus_running = !app->save.focus_running;
      if (app->save.focus_running) app->save.focus_end = now + app->save.focus_minutes * 60;
      snprintf(app->notice, sizeof(app->notice), "%s", app->save.focus_running ? (zh(app) ? "专注行动已开始。" : "Operation started.") : (zh(app) ? "专注行动已中止。" : "Operation aborted."));
    }
  } else if (app->screen == DT_SCREEN_TASKS) {
    if (key == DT_KEY_UP) app->cursor = (app->cursor + DT_TASK_COUNT - 1) % DT_TASK_COUNT;
    if (key == DT_KEY_DOWN) app->cursor = (app->cursor + 1) % DT_TASK_COUNT;
    if (key == DT_KEY_OK) app->save.tasks[app->cursor].completed = !app->save.tasks[app->cursor].completed;
  } else if (app->screen == DT_SCREEN_EVENT && key == DT_KEY_OK) {
    unsigned int index = next_random(app) % 5u;
    int reward = index == 2 ? 37 : (index == 4 ? 120 : 0);
    app->save.coins += reward;
    app->save.pet_mood += index == 0 ? 6 : 2;
    if (app->save.pet_mood > 100) app->save.pet_mood = 100;
    if (reward) snprintf(app->notice, sizeof(app->notice), zh(app) ? "事件奖励：+%d龙门币" : "Reward: +%d LMD", reward);
    else snprintf(app->notice, sizeof(app->notice), "%s", zh(app) ? event_zh[index] : event_en[index]);
  } else if (app->screen == DT_SCREEN_PET) {
    if (key == DT_KEY_UP) app->cursor = (app->cursor + 3) % 4;
    if (key == DT_KEY_DOWN) app->cursor = (app->cursor + 1) % 4;
    if (key == DT_KEY_OK && app->cursor == 0) {
      if (app->save.coins < 100) snprintf(app->notice, sizeof(app->notice), "%s", zh(app) ? "龙门币不足。" : "Not enough LMD.");
      else { app->save.coins -= 100; app->save.pet_hunger += 28; app->save.pet_mood += 3; snprintf(app->notice, sizeof(app->notice), "%s", zh(app) ? "宠物吃得很开心。" : "Pet enjoyed the meal."); }
    } else if (key == DT_KEY_OK && app->cursor == 1) {
      if (app->save.pet_energy < 12) snprintf(app->notice, sizeof(app->notice), "%s", zh(app) ? "宠物需要先休息。" : "Pet needs to rest.");
      else { app->save.pet_energy -= 12; app->save.pet_mood += 15; app->save.pet_bond += 5; app->save.pet_xp += 3; snprintf(app->notice, sizeof(app->notice), "%s", zh(app) ? "亲密度提升了。" : "Bond increased."); }
    } else if (key == DT_KEY_OK && app->cursor == 2) {
      app->save.pet_energy += 30; app->save.pet_hunger -= 5; snprintf(app->notice, sizeof(app->notice), "%s", zh(app) ? "宠物睡着了。" : "Pet is sleeping.");
    } else if (key == DT_KEY_OK && app->cursor == 3) {
      if (app->save.pet_energy < 25 || app->save.pet_hunger < 20) snprintf(app->notice, sizeof(app->notice), "%s", zh(app) ? "当前状态不能探索。" : "Pet cannot explore now.");
      else { int gain = 50 + (int)(next_random(app) % 251); app->save.pet_energy -= 25; app->save.pet_hunger -= 12; app->save.coins += gain; app->save.pet_xp += 8; snprintf(app->notice, sizeof(app->notice), zh(app) ? "探索带回%d龙门币" : "Exploration: +%d LMD", gain); }
    }
    if (app->save.pet_mood > 100) app->save.pet_mood = 100;
    if (app->save.pet_hunger > 100) app->save.pet_hunger = 100;
    if (app->save.pet_energy > 100) app->save.pet_energy = 100;
    if (app->save.pet_bond > 100) app->save.pet_bond = 100;
  } else if (app->screen == DT_SCREEN_LANGUAGE) {
    if (key == DT_KEY_UP || key == DT_KEY_DOWN) snprintf(app->save.language, sizeof(app->save.language), "%s", zh(app) ? "en" : "zh");
    if (key == DT_KEY_OK) { app->screen = DT_SCREEN_HOME; app->cursor = 0; }
  } else if (app->screen == DT_SCREEN_EXIT && key == DT_KEY_OK) app->quit = 1;
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
  int cn = zh(app);
  if (!capacity) return;
  out[0] = '\0';
  append(out, capacity, &used, "RHODES DAILY TERMINAL\nLV.%02d   LMD %06d\n", app->save.level, app->save.coins);
  if (app->screen == DT_SCREEN_HOME) {
    append(out, capacity, &used, "\n%s  %d/%d\n", cn ? "任务" : "MISSIONS", completed(app), DT_TASK_COUNT);
    for (int i = 0; i < MENU_COUNT; ++i) append(out, capacity, &used, "%c %s\n", i == app->cursor ? '>' : ' ', cn ? menu_zh[i] : menu_en[i]);
  } else if (app->screen == DT_SCREEN_FOCUS) {
    long remain = app->save.focus_running ? (long)(app->save.focus_end - now) : app->save.focus_minutes * 60L;
    if (remain < 0) remain = 0;
    append(out, capacity, &used, "\n%s\n\n%02ld:%02ld\n\n%s\n", cn ? "专注行动" : "FOCUS", remain / 60, remain % 60, app->save.focus_running ? (cn ? "KEY3 中止" : "KEY3 ABORT") : (cn ? "KEY1/2 调整  KEY3 开始" : "KEY1/2 SET  KEY3 START"));
  } else if (app->screen == DT_SCREEN_TASKS) {
    append(out, capacity, &used, "\n%s\n\n", cn ? "每日任务" : "MISSIONS");
    for (int i = 0; i < DT_TASK_COUNT; ++i) append(out, capacity, &used, "%c[%c] %s\n", i == app->cursor ? '>' : ' ', app->save.tasks[i].completed ? 'x' : ' ', cn ? task_zh[i] : task_en[i]);
  } else if (app->screen == DT_SCREEN_EVENT) {
    append(out, capacity, &used, "\n%s\n\n%s\n", cn ? "区域事件" : "FIELD EVENT", cn ? "按KEY3扫描当前区域" : "KEY3 SCAN LOCAL SECTOR");
  } else if (app->screen == DT_SCREEN_PET) {
    static const char *act_en[] = {"FEED - 100 LMD", "PLAY", "REST", "EXPLORE"};
    static const char *act_zh[] = {"喂食 - 100龙门币", "玩耍", "休息", "探索"};
    const char *stage = app->save.pet_xp >= 120 ? (cn ? "精英源石兽" : "ELITE ORIGINIUM BEAST") : app->save.pet_xp >= 40 ? (cn ? "活跃源石虫" : "ACTIVE ORIGINIUM SLUG") : (cn ? "幼年源石虫" : "YOUNG ORIGINIUM SLUG");
    append(out, capacity, &used, "\n%s\n%s\n%s%d  %s%d\n%s%d  %s%d\n\n", cn ? "电子宠物" : "PET", stage, cn ? "心情" : "MOOD ", app->save.pet_mood, cn ? "饱食" : "HUNGER ", app->save.pet_hunger, cn ? "精力" : "ENERGY ", app->save.pet_energy, cn ? "亲密" : "BOND ", app->save.pet_bond);
    for (int i = 0; i < 4; ++i) append(out, capacity, &used, "%c %s\n", i == app->cursor ? '>' : ' ', cn ? act_zh[i] : act_en[i]);
  } else if (app->screen == DT_SCREEN_STATUS) {
    append(out, capacity, &used, "\n%s\n\n%s: %d\n%s: %d/%d\n%s: %d%%\n%s: %d/%d\n", cn ? "干员状态" : "STATUS", cn ? "等级" : "LEVEL", app->save.level, cn ? "经验" : "XP", app->save.xp, app->save.level * 100, cn ? "宠物心情" : "PET MOOD", app->save.pet_mood, cn ? "任务" : "MISSIONS", completed(app), DT_TASK_COUNT);
  } else if (app->screen == DT_SCREEN_LANGUAGE) {
    append(out, capacity, &used, "\nLANGUAGE / 语言\n\n%s 简体中文\n%s ENGLISH\n\n%s\n", cn ? ">" : " ", cn ? " " : ">", cn ? "KEY1/2 切换  KEY3 确认" : "KEY1/2 SWITCH  KEY3 OK");
  } else {
    append(out, capacity, &used, "\n%s\n\n%s\n\n%s\n%s\n", cn ? "退出程序" : "EXIT", cn ? "结束本次终端会话？" : "END TERMINAL SESSION?", cn ? "KEY3 保存并退出" : "KEY3 SAVE & EXIT", cn ? "KEY4 取消" : "KEY4 CANCEL");
  }
  if (app->notice[0]) append(out, capacity, &used, "\nPRTS> %s\n", app->notice);
  append(out, capacity, &used, "\nKEY1/2 %s  KEY3 %s  KEY4 %s\n", cn ? "选择" : "MOVE", cn ? "确认" : "OK", cn ? "返回" : "BACK");
}
