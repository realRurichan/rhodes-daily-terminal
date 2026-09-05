#include "platform.h"
#include "epass_game.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static game_platform_t native_platform;

int dt_platform_init(DtPlatform *platform) {
  memset(&native_platform, 0, sizeof(native_platform));
  platform->input_fd = -1;
  platform->interactive = 1;
  return game_platform_init(&native_platform) ? 1 : 0;
}

void dt_platform_shutdown(DtPlatform *platform) {
  (void)platform;
  game_platform_destroy(&native_platform);
}

DtKey dt_platform_read_key(DtPlatform *platform, int timeout_ms) {
  (void)platform;
  game_input_update(&native_platform);
  if (game_key_pressed(&native_platform, GAME_KEY_UP) || game_key_repeated(&native_platform, GAME_KEY_UP)) return DT_KEY_UP;
  if (game_key_pressed(&native_platform, GAME_KEY_DOWN) || game_key_repeated(&native_platform, GAME_KEY_DOWN)) return DT_KEY_DOWN;
  if (game_key_pressed(&native_platform, GAME_KEY_OK)) return DT_KEY_OK;
  if (game_key_pressed(&native_platform, GAME_KEY_BACK)) return DT_KEY_BACK;
  game_platform_idle(&native_platform, (unsigned int)timeout_ms);
  return DT_KEY_NONE;
}

static uint32_t line_color(const char *line) {
  if (strstr(line, "PRTS>")) return 0xff73f4c7u;
  if (strstr(line, "RHODES ISLAND")) return 0xfff5c451u;
  if (line[0] == ' ' && line[1] == '>') return 0xff121a20u;
  return 0xffd9e1e5u;
}

static void draw_text_line(game_framebuffer_t *fb, int y, char *line) {
  int selected = line[0] == ' ' && line[1] == '>';
  if (selected) game_draw_rect(fb, 12, y - 4, 336, 24, 0xfff5c451u);
  game_draw_text(fb, 18, y, line, 1, line_color(line));
}

void dt_platform_draw(const char *frame) {
  game_framebuffer_t fb;
  if (!game_platform_acquire_frame(&native_platform, &fb)) return;
  game_draw_fill(&fb, 0xff0b1014u);
  game_draw_rect(&fb, 0, 0, 360, 8, 0xfff5c451u);
  game_draw_rect(&fb, 0, 104, 360, 2, 0xff34434bu);

  char copy[4096];
  snprintf(copy, sizeof(copy), "%s", frame);
  int y = 24;
  char *cursor = copy;
  while (*cursor && y < 606) {
    char *end = strchr(cursor, '\n');
    if (end) *end = '\0';
    if (strncmp(cursor, "===", 3) != 0 && cursor[0] != '\0') {
      char clipped[55];
      snprintf(clipped, sizeof(clipped), "%.54s", cursor);
      draw_text_line(&fb, y, clipped);
      y += 28;
    } else if (cursor[0] == '\0') {
      y += 14;
    }
    if (!end) break;
    cursor = end + 1;
  }
  game_draw_rect(&fb, 0, 622, 360, 18, 0xfff5c451u);
  game_platform_present(&native_platform);
}

const char *dt_platform_data_path(void) {
  const char *path = getenv("EPASS_SAVE_PATH");
  return path ? path : "/root/.daily_terminal.sav";
}
