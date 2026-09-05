#ifndef DAILY_TERMINAL_STORAGE_H
#define DAILY_TERMINAL_STORAGE_H

#include "app.h"

int dt_save_load(const char *path, DtSave *save);
int dt_save_write(const char *path, const DtSave *save);

#endif

