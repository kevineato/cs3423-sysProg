#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define LFLAG 0x4
#define AFLAG 0x2
#define RFLAG 0x1
#define MAX_NAME 500

void errExit(const char szFmt[], ...);
void xDir(const char *dName, uint8_t flags, int depth);
void indPrint(const char *str, int indent);

int main(int argc, char *argv[]) {
  int o;
  uint8_t larFlags = 0;
  DIR *d;
  struct dirent *dEnt;

  /* Check if at least one arg */
  if (argc < 2) {
    fprintf(stderr, "Usage: %s DIR [-lar]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  optind = 2;
  while ((o = getopt(argc, argv, "lar")) != -1) {
    switch(o) {
      case 'l':
        larFlags |= LFLAG;
        break;
      case 'a':
        larFlags |= AFLAG;
        break;
      case 'r':
        larFlags |= RFLAG;
        break;
      case '?':
        fprintf(stderr, "Unrecognized option '-%c'\n", optopt);
    }
  }

  xDir(argv[1], larFlags, 1);
  exit(EXIT_SUCCESS);
}

void xDir(const char *dName, uint8_t flags, int depth) {
  int indent = depth * 4;
  DIR *d;
  struct dirent *dEnt;
  struct stat stBuf;

  uint8_t lFlag = LFLAG & flags;
  uint8_t aFlag = AFLAG & flags;
  uint8_t rFlag = RFLAG & flags;

  const char *revSearchSlash = strrchr(dName, '/');
  const char *baseName = revSearchSlash ? revSearchSlash + 1 : dName;
  char *path = (char *) malloc(MAX_NAME);

  d = opendir(dName);
  if (d == NULL)
    errExit("Could not open '%s': %s", dName, strerror(errno));

  printf("%*s%s :\n", (depth - 1) * 4, "", baseName);
  while ((dEnt = readdir(d)) != NULL) {
    if (!aFlag && dEnt->d_name[0] == '.')
      continue;

    snprintf(path, MAX_NAME, "%s/%s", dName, dEnt->d_name);

    if (stat(path, &stBuf) == -1)
      continue;

    if (rFlag && S_ISDIR(stBuf.st_mode) && dEnt->d_name[0] != '.') {
      xDir(path, flags, depth + 1);
      continue;
    }

    if (lFlag) {
      char *listFormat = (char *) malloc(MAX_NAME);

      char type;
      if (S_ISREG(stBuf.st_mode))
        type = 'F';
      else if (S_ISDIR(stBuf.st_mode))
        type = 'D';
      else if (S_ISFIFO(stBuf.st_mode))
        type = 'P';
      else if (S_ISLNK(stBuf.st_mode))
        type = 'L';
      else
        type = '?';

      snprintf(listFormat, MAX_NAME, "%s %c %ld blks %ld bytes", dEnt->d_name, type, stBuf.st_blocks, stBuf.st_size);
      indPrint(listFormat, indent);

      free(listFormat);
    } else {
      indPrint(dEnt->d_name, indent);
    }
  }

  free(path);
  closedir(d);
}

void indPrint(const char *str, int indent) {
  printf("%*s%s\n", indent, "", str);
}
