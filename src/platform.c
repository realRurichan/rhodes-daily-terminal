#include "platform.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>
static struct termios original_term;
#endif

int dt_platform_init(DtPlatform *platform) {
  platform->input_fd = -1;
  platform->interactive = 1;
#ifdef _WIN32
  return 1;
#else
  const char *device = getenv("EPASS_INPUT_DEVICE");
  if (!device) device = "/dev/input/event0";
  platform->input_fd = open(device, O_RDONLY | O_NONBLOCK);
  if (isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &original_term) == 0) {
    struct termios raw = original_term;
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
  }
  return 1;
#endif
}

void dt_platform_shutdown(DtPlatform *platform) {
#ifndef _WIN32
  if (platform->input_fd >= 0) close(platform->input_fd);
  if (isatty(STDIN_FILENO)) tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
#else
  (void)platform;
#endif
}

static DtKey map_char(int c) {
  if (c == 'w' || c == 'W') return DT_KEY_UP;
  if (c == 's' || c == 'S') return DT_KEY_DOWN;
  if (c == '\r' || c == '\n' || c == 'e' || c == 'E') return DT_KEY_OK;
  if (c == 'q' || c == 'Q' || c == 27) return DT_KEY_BACK;
  return DT_KEY_NONE;
}

DtKey dt_platform_read_key(DtPlatform *platform, int timeout_ms) {
#ifdef _WIN32
  int elapsed = 0;
  while (elapsed < timeout_ms) {
    if (_kbhit()) return map_char(_getch());
    Sleep(20); elapsed += 20;
  }
#else
  if (platform->input_fd >= 0) {
    struct pollfd pfd = {platform->input_fd, POLLIN, 0};
    if (poll(&pfd, 1, timeout_ms) > 0) {
      struct input_event event;
      if (read(platform->input_fd, &event, sizeof(event)) == sizeof(event) && event.type == EV_KEY && event.value == 1) {
        if (event.code == KEY_UP || event.code == KEY_1) return DT_KEY_UP;
        if (event.code == KEY_DOWN || event.code == KEY_2) return DT_KEY_DOWN;
        if (event.code == KEY_ENTER || event.code == KEY_3) return DT_KEY_OK;
        if (event.code == KEY_ESC || event.code == KEY_4) return DT_KEY_BACK;
      }
    }
  } else {
    struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
    if (poll(&pfd, 1, timeout_ms) > 0) { unsigned char c; if (read(STDIN_FILENO, &c, 1) == 1) return map_char(c); }
  }
#endif
  return DT_KEY_NONE;
}

void dt_platform_draw(const char *frame) {
  printf("\033[2J\033[H%s", frame);
  fflush(stdout);
}

const char *dt_platform_data_path(void) {
  const char *path = getenv("EPASS_SAVE_PATH");
  return path ? path : "daily_terminal.sav";
}

