#include "platform.h"
#include "epass_game.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static game_platform_t native_platform;
static FT_Library font_library;
static FT_Face font_face;

#ifndef DT_FONT_PATH
#define DT_FONT_PATH "/usr/share/fonts/epass/SourceHanSansSC-Regular.otf"
#endif

int dt_platform_init(DtPlatform *platform) {
  memset(&native_platform, 0, sizeof(native_platform));
  platform->input_fd = -1;
  platform->interactive = 1;
  if (!game_platform_init(&native_platform)) return 0;
  if (FT_Init_FreeType(&font_library) == 0 &&
      FT_New_Face(font_library, DT_FONT_PATH, 0, &font_face) == 0)
    FT_Set_Pixel_Sizes(font_face, 0, 22);
  return 1;
}

void dt_platform_shutdown(DtPlatform *platform) {
  (void)platform;
  if (font_face) FT_Done_Face(font_face);
  if (font_library) FT_Done_FreeType(font_library);
  font_face = NULL;
  font_library = NULL;
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
  if (line[0] == '>') return 0xff121a20u;
  return 0xffd9e1e5u;
}

static unsigned int next_codepoint(const unsigned char **text) {
  const unsigned char *s = *text;
  unsigned int code;
  if (*s < 0x80) { code = *s++; }
  else if ((*s & 0xe0) == 0xc0 && s[1]) { code = ((s[0] & 0x1f) << 6) | (s[1] & 0x3f); s += 2; }
  else if ((*s & 0xf0) == 0xe0 && s[1] && s[2]) { code = ((s[0] & 0x0f) << 12) | ((s[1] & 0x3f) << 6) | (s[2] & 0x3f); s += 3; }
  else if ((*s & 0xf8) == 0xf0 && s[1] && s[2] && s[3]) { code = ((s[0] & 7) << 18) | ((s[1] & 0x3f) << 12) | ((s[2] & 0x3f) << 6) | (s[3] & 0x3f); s += 4; }
  else { code = '?'; s++; }
  *text = s;
  return code;
}

static void draw_utf8(game_framebuffer_t *fb, int x, int y, const char *line, uint32_t color) {
  if (!font_face) { game_draw_text(fb, x, y, line, 2, color); return; }
  const unsigned char *text = (const unsigned char *)line;
  while (*text && x < 346) {
    unsigned int code = next_codepoint(&text);
    if (FT_Load_Char(font_face, code, FT_LOAD_RENDER) != 0) continue;
    FT_GlyphSlot glyph = font_face->glyph;
    FT_Bitmap *bitmap = &glyph->bitmap;
    int left = x + glyph->bitmap_left;
    int top = y + 23 - glyph->bitmap_top;
    for (unsigned int row = 0; row < bitmap->rows; ++row)
      for (unsigned int col = 0; col < bitmap->width; ++col)
        if (bitmap->buffer[row * bitmap->pitch + col] > 72)
          game_draw_rect(fb, left + (int)col, top + (int)row, 1, 1, color);
    x += (int)(glyph->advance.x >> 6);
  }
}

static void draw_text_line(game_framebuffer_t *fb, int y, char *line) {
  int selected = line[0] == '>';
  if (selected) game_draw_rect(fb, 10, y - 4, 340, 30, 0xfff5c451u);
  draw_utf8(fb, 16, y, line, line_color(line));
}

void dt_platform_draw(const char *frame) {
  game_framebuffer_t fb;
  if (!game_platform_acquire_frame(&native_platform, &fb)) return;
  game_draw_fill(&fb, 0xff0b1014u);
  game_draw_rect(&fb, 0, 0, 360, 8, 0xfff5c451u);
  game_draw_rect(&fb, 0, 104, 360, 2, 0xff34434bu);

  char copy[4096];
  snprintf(copy, sizeof(copy), "%s", frame);
  int y = 22;
  char *cursor = copy;
  while (*cursor && y < 606) {
    char *end = strchr(cursor, '\n');
    if (end) *end = '\0';
    if (strncmp(cursor, "===", 3) != 0 && cursor[0] != '\0') {
      draw_text_line(&fb, y, cursor);
      y += 34;
    } else if (cursor[0] == '\0') {
      y += 16;
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
