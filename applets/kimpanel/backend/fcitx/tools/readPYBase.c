#include <stdio.h>

#include "py.h"
#include "pyTools.h"

void usage();

int main(int argc, char **argv)
{
  FILE *fi;
  int i, PYFACount;
  char *pybase_mb;
  struct _HZMap *HZMap;

  if (argc > 2)
    usage();

  pybase_mb = strdup((argc > 1) ? argv[1] : (PKGDATADIR "/data/" PY_BASE_FILE));
  fi = tryopen(pybase_mb);

  PYFACount = LoadPYBase(fi, &HZMap);
  if (PYFACount > 0)
  {
#if 0
    for (i = 0; i < PYFACount; ++i)
    {
      printf("%s: ", HZMap[i].Map);
      fwrite(HZMap[i].HZ, 2, HZMap[i].BaseCount, stdout);
      printf("\n\n");
    }
#else
    for (i = 0; i < PYFACount; ++i)
    {
      int j;
      printf("%s: HZ Index\n", HZMap[i].Map);
      for (j = 0; j < HZMap[i].BaseCount / 2; ++j)
      {
        printf("    ");
        fwrite(HZMap[i].HZ + 2 * j, 2, 1, stdout);
        printf(" %5d\n", *(HZMap[i].Index + 2 * j));
      }
      printf("\n");
    }
#endif
  }

  return 0;
}

void usage()
{
  puts(
"readPYBase - read pybase.mb file and display its contents\n"
"\n"
"  usage: readPYBase [<pybase.mb>]\n"
"\n"
"  <pybase.mb>    full path to the file, usually\n"
"                 " PKGDATADIR "/data/" PY_BASE_FILE "\n"
"                 if not specified, defaults to\n"
"                 " PKGDATADIR "/data/" PY_BASE_FILE "\n"
"\n"
  );
  exit(1);
  return;
}

