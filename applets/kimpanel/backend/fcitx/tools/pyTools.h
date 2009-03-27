#ifndef _PY_TOOLS_H
#define _PY_TOOLS_H

struct _PYMB
{
  int PYFAIndex;
  char HZ[3];
  int UserPhraseCount;
  struct
  {
    int Length;
    char *Map;
    char *Phrase;
    int Index;
    int Hit;
  } *UserPhrase;
};

struct _HZMap
{
  char Map[3];
  int BaseCount;
  char *HZ;
  int *Index;
};

int LoadPYBase(FILE *, struct _HZMap **);
void LoadPYMB(FILE *, struct _PYMB **);

char *getuserfile(char *, char *);

FILE *tryopen(char *);

#endif /* _PY_TOOLS_H */

