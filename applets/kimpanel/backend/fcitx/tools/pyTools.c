#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pyTools.h"

void LoadPYMB(FILE *fi, struct _PYMB **pPYMB)
{
  struct _PYMB *PYMB;
  int i, j, r, n, t, t2;

  /* Is there a way to avoid reading the whole file twice? */

  /* First Pass: Determine the size of the PYMB array to be created */

  n = 0;
  while (1)
  {
    r = fread(&t, sizeof (int), 1, fi);
    if (!r)
      break;
    ++n;

    fseek(fi, 2, SEEK_CUR);
    fread(&t, sizeof (int), 1, fi);

    for (i = 0; i < t; ++i)
    {
      fread(&t2, sizeof (int), 1, fi);
      fseek(fi, 2 * t2 + 2 * sizeof (int), SEEK_CUR);
    }
  }

  /* Second Pass: Actually read the data */

  fseek(fi, 0, SEEK_SET);

  *pPYMB = PYMB = malloc(sizeof (*PYMB) * (n + 1));

  for (i = 0; i < n; ++i)
  {
    r = fread(&(PYMB[i].PYFAIndex), sizeof (int), 1, fi);

    fread(PYMB[i].HZ, sizeof (char) * 2, 1, fi);
    PYMB[i].HZ[2] = '\0';

    fread(&(PYMB[i].UserPhraseCount), sizeof (int), 1, fi);
    PYMB[i].UserPhrase = malloc(sizeof(*(PYMB[i].UserPhrase)) * PYMB[i].UserPhraseCount);

#define PU(i,j) (PYMB[(i)].UserPhrase[(j)])
    for (j = 0; j < PYMB[i].UserPhraseCount; ++j)
    {
      fread(&(PU(i,j).Length), sizeof (int), 1, fi);

      PU(i,j).Map = malloc(sizeof (char) * PU(i,j).Length + 1);
      fread(PU(i,j).Map, sizeof (char) * PU(i,j).Length, 1, fi);
      PU(i,j).Map[PU(i,j).Length] = '\0';

      PU(i,j).Phrase = malloc(sizeof (char) * PU(i,j).Length + 1);
      fread(PU(i,j).Phrase, sizeof (char) * PU(i,j).Length, 1, fi);
      PU(i,j).Phrase[PU(i,j).Length] = '\0';

      fread(&(PU(i,j).Index), sizeof (int), 1, fi);

      fread(&(PU(i,j).Hit), sizeof (int), 1, fi);
    }
#undef PU
  }
  PYMB[n].HZ[0] = '\0';

  return;
}

int LoadPYBase(FILE *fi, struct _HZMap **pHZMap)
{
  int i, j, r, PYFACount;
  struct _HZMap *HZMap;

  r = fread(&PYFACount, sizeof (int), 1, fi);
  if (!r)
    return 0;

  *pHZMap = HZMap = malloc(sizeof (*HZMap) * (PYFACount + 1));
  for (i = 0; i < PYFACount; ++i)
  {
    fread(HZMap[i].Map, 2, 1, fi);
    HZMap[i].Map[2] = '\0';

    fread(&(HZMap[i].BaseCount), sizeof (int), 1, fi);
    HZMap[i].HZ = malloc(2 * HZMap[i].BaseCount);
    HZMap[i].Index = malloc(sizeof (int) * HZMap[i].BaseCount);

    for (j = 0; j < HZMap[i].BaseCount; ++j)
    {
      fread(HZMap[i].HZ + j * 2, 2, 1, fi);
      fread(HZMap[i].Index + j, sizeof (int), 1, fi);
    }
  }
  HZMap[i].Map[0] = '\0';

  return PYFACount;
}

FILE *tryopen(char *filename)
{
  FILE *fi;

  fi = fopen(filename, "r");
  if (!fi)
  {
    perror("fopen");
    fprintf(stderr, "Can't open file `%s' for reading\n", filename);
    exit(1);
  }

  return fi;
}

char *getuserfile(char *name, char *given)
{
  char *filename, *home;

  if (given[0])
    filename = strdup(given);
  else
  {
    home = getenv("HOME");
    if (!home)
      home = strdup("~");
    filename = malloc(strlen(home) + strlen("/.fcitx/") + strlen(name) + 1);
    strcpy(filename, home);
    strcat(filename, "/.fcitx/");
    strcat(filename, name);
  }

  return filename;
}

