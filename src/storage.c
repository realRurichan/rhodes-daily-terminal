#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "storage.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#define DT_SAVE_MAGIC 0x44545031u
typedef struct { unsigned int magic; DtSave save; } SaveFile;

int dt_save_load(const char *path, DtSave *save) {
  SaveFile file;
  FILE *fp = fopen(path, "rb");
  if (!fp) return 0;
  int ok = fread(&file, sizeof(file), 1, fp) == 1 && file.magic == DT_SAVE_MAGIC;
  fclose(fp);
  if (ok) memcpy(save, &file.save, sizeof(*save));
  return ok;
}

int dt_save_write(const char *path, const DtSave *save) {
  SaveFile file = {DT_SAVE_MAGIC, {0}};
  char temporary[512];
  if (snprintf(temporary, sizeof(temporary), "%s.tmp", path) < 0 ||
      strlen(path) + 4 >= sizeof(temporary)) return 0;
  memcpy(&file.save, save, sizeof(*save));
  FILE *fp = fopen(temporary, "wb");
  if (!fp) return 0;
  int ok = fwrite(&file, sizeof(file), 1, fp) == 1;
  if (ok) ok = fflush(fp) == 0;
#ifdef _WIN32
  if (ok) ok = _commit(_fileno(fp)) == 0;
#else
  if (ok) ok = fsync(fileno(fp)) == 0;
#endif
  if (fclose(fp) != 0) ok = 0;
  if (!ok) { remove(temporary); return 0; }
#ifdef _WIN32
  remove(path);
#endif
  if (rename(temporary, path) != 0) { remove(temporary); return 0; }
  return 1;
}
