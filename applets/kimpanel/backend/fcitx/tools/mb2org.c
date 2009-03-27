#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pyParser.h"
#include "pyMapTable.h"
#include "PYFA.h"
#include "sp.h"
#include "pyTools.h"

/* Bad programming practice :( */
Bool bFullPY;
Bool bSingleHZMode;

void usage();
char *HZToPY(struct _HZMap *, char []);

int main(int argc, char **argv)
{
  FILE *fi, *fi2;
  int i, j, k;
  char *pyusrphrase_mb, *pybase_mb, *HZPY, tMap[3], tPY[10];
  struct _HZMap *HZMap;
  struct _PYMB *PYMB;

  if (argc > 3)
    usage();

  pyusrphrase_mb = getuserfile(PY_USERPHRASE_FILE, (argc > 1) ? argv[1] : "");
  fi = tryopen(pyusrphrase_mb);

  pybase_mb = strdup((argc > 2) ? argv[2] : (PKGDATADIR "/data/" PY_BASE_FILE));
  fi2 = tryopen(pybase_mb);

  LoadPYMB(fi, &PYMB);
  LoadPYBase(fi2, &HZMap);

  for (i = 0; PYMB[i].HZ[0]; ++i)
  {
    for (j = 0; j < PYMB[i].UserPhraseCount; ++j)
    {
      HZPY = HZToPY(&(HZMap[PYMB[i].PYFAIndex]), PYMB[i].HZ);
      printf("%s", HZPY);

      for (k = 0; k < PYMB[i].UserPhrase[j].Length / 2; ++k)
      {
        memcpy(tMap, PYMB[i].UserPhrase[j].Map + 2 * k, 2);
        tMap[2] = '\0';
        tPY[0] = '\0';
        if (!MapToPY(tMap, tPY))
          strcpy(tPY, "'*");
        printf("'%s", tPY);
      }
      printf(" %s%s\n", PYMB[i].HZ, PYMB[i].UserPhrase[j].Phrase);

      free(HZPY);
    }
    printf("\n");
  }

  return 0;
}

/*
  This function takes a HanZi (HZ) and returns a PinYin (PY) string.
  If no match is found, "*" is returned.
*/

char *HZToPY(struct _HZMap *pHZMap1, char HZ[3])
{
  int i;
  char Map[3], tPY[10];

  Map[0] = '\0';
  for (i = 0; i < pHZMap1->BaseCount; ++i)
    if (memcmp(HZ, pHZMap1->HZ + 2 * i, 2))
    {
      strcpy(Map, pHZMap1->Map);
      break;
    }

  if (!Map[0] || !MapToPY(Map, tPY))
    strcpy(tPY, "*");

  return strdup(tPY);
}

void usage()
{
  puts(
"mb2org - Convert .mb file to .org file (SEE NOTES BELOW)\n"
"\n"
"  usage: mb2org [<pyusrphrase.mb>] [<pybase.mb>]\n"
"\n"
"  <pyusrphrase.mb>   this is the .mb file to be decoded, usually this is\n"
"                     ~/.fcitx/" PY_USERPHRASE_FILE "\n"
"                     if not specified, defaults to\n"
"                     ~/.fcitx/" PY_USERPHRASE_FILE "\n"
"  <pybase.mb>        this is the pybase.mb file used to determine the\n"
"                     of the first character in HZ. Usually, this is\n"
"                     " PKGDATADIR "/data/" PY_BASE_FILE "\n"
"                     if not specified, defaults to\n"
"                     " PKGDATADIR "/data/" PY_BASE_FILE "\n"
"\n"
"NOTES:\n"
"1. If no match is found for a particular HZ, then the pinyin for that HZ\n"
"   will be `*'.\n"
"2. Always check the produced output for errors.\n"
  );
  exit(1);
  return;
}

