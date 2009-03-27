#include <stdio.h>
#include <stdlib.h>

#include "py.h"
#include "pyTools.h"

void usage();

int main(int argc, char **argv)
{
  FILE *fi;
  int i, j;
  char *pyusrphrase_mb;
  struct _PYMB *PYMB;

  if (argc > 3)
    usage();

  pyusrphrase_mb = getuserfile(PY_USERPHRASE_FILE, (argc > 1) ? argv[1] : "");
  fi = tryopen(pyusrphrase_mb);
  LoadPYMB(fi, &PYMB);

  for (i = 0; PYMB[i].HZ[0]; ++i)
  {
    printf("PYFAIndex: %d\n", PYMB[i].PYFAIndex);
    printf("HZ: %s\n", PYMB[i].HZ);
    printf("UserPhraseCount: %d\n", PYMB[i].UserPhraseCount);

    for (j = 0; j < PYMB[i].UserPhraseCount; ++j)
    {
      printf("+-Length: %d\n", PYMB[i].UserPhrase[j].Length);
      printf("| Map: %s\n", PYMB[i].UserPhrase[j].Map);
      printf("| Phrase: %s\n", PYMB[i].UserPhrase[j].Phrase);
      printf("| Index: %d\n", PYMB[i].UserPhrase[j].Index);
      printf("| Hit: %d\n", PYMB[i].UserPhrase[j].Hit);
    }
    printf("\n");
  }

  return 0;
}

void usage()
{
  puts(
"readPYMB - read data from a pinyin .mb file and display its meaning\n"
"\n"
"  usage: readPYMB <mbfile>\n"
"\n"
"  <mbfile>    MB (MaBiao) file to be read, usually this is\n"
"              ~/.fcitx/" PY_USERPHRASE_FILE "\n"
"              if not specified, defaults to\n"
"              ~/.fcitx/" PY_USERPHRASE_FILE "\n"
"\n"
"  The MB file can either be a user's MB file (~/.fcitx/pyuserphrase.mb),\n"
"  or the system phrase pinyin MB file (/usr/share/fcitx/data/pyphrase.mb.\n"
  );
  exit(1);
  return;
}

