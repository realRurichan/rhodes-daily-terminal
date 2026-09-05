#include "storage.h"

#include <stdio.h>
#include <string.h>

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
  memcpy(&file.save, save, sizeof(*save));
  FILE *fp = fopen(path, "wb");
  if (!fp) return 0;
  int ok = fwrite(&file, sizeof(file), 1, fp) == 1;
  ok = fclose(fp) == 0 && ok;
  return ok;
}

