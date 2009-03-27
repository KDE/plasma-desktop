/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * 将拼音加加的用户词库转换为小企鹅输入法的词库
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include "pyParser.h"
#include "pyMapTable.h"
#include "PYFA.h"
#include "sp.h"

Bool            bSingleHZMode = False;
Bool            bFullPY = False;

typedef struct {
    char            strPY[7];
    char            strHZ[3];
} PYBASE;

PYBASE          pyBase[30000];
uint            iPYCount = 0;

Bool LoadPYBase (void)
{
    FILE           *fp;

    fp = fopen ("gbpy.org", "rt");
    if (!fp) {
	fprintf (stderr, "Can not open ../data/gbpy.org!\n");
	return False;
    }

    while (!feof (fp)) {
	fscanf (fp, "%s", pyBase[iPYCount].strPY);
	fscanf (fp, "%s\n", pyBase[iPYCount].strHZ);
	iPYCount++;
    }

    fclose (fp);
    return True;
}

int GetPYByHZ (char *strHZ, char *strPY)
{
    uint            i;
    int             iRet;

    strPY[0] = '\0';
    iRet = 0;
    for (i = 0; i < iPYCount; i++) {
	if (!strcmp (strHZ, pyBase[i].strHZ)) {
	    iRet++;
	    strcat (strPY, pyBase[i].strPY);
	}
    }

    return iRet;
}

int main (int argc, char *argv[])
{
    FILE           *fp;
    char            str[100];
    char            strHZs[50], strHZ[3];
    char            strPYs[100], strPY[7], strPYTemp[100];
    char           *pstr, *pHZ, *pPY;
    int             i, s, t;

    if (argc != 2) {
	fprintf (stderr, "Usage: %s <PYJJ User Phrase File>\n", argv[0]);
	exit (-1);
    }

    fp = fopen (argv[1], "rt");
    if (!fp) {
	fprintf (stderr, "Can't open file %s\n", argv[1]);
	exit (1);
    }

    if (!LoadPYBase ())
	exit (-1);

    strHZ[2] = '\0';
    s = 0;
    t = 0;
    for (;;) {
	if (!fgets (str, 100, fp))
	    break;

	i = strlen (str) - 1;
	while ((i >= 0) && (str[i] == ' ' || str[i] == '\n' || str[i] == '\r'))
	    str[i--] = '\0';

	pstr = str;
	if (*pstr == ' ')
	    pstr++;
	s++;
	strPYs[0] = '\0';
	strHZs[0] = '\0';
	while (*pstr) {
	    if (!isprint (*pstr)) {	//是汉字
		pHZ = strHZ;
		strHZ[0] = *pstr++;
		strHZ[1] = *pstr++;
		strcat (strHZs, strHZ);
		if (isalpha (*pstr)) {
		    pPY = strPY;
		    while (isalpha (*pstr))
			*pPY++ = *pstr++;
		    *pPY = '\0';
		    strcat (strPYs, "'");
		    strcat (strPYs, strPY);
		}
		else {
		    int             y;

		    y = GetPYByHZ (strHZ, strPYTemp);
		    if (y != 1) {	//出错了
			fprintf (stderr, "%d - Can not process: %s (%s %d)\n", s, str, strHZ, y);
			t++;
			goto _next;
		    }
		    strcat (strPYs, "'");
		    strcat (strPYs, strPYTemp);
		}
	    }
	}

	if (strlen (strHZs) > 20) {
	    fprintf (stderr, "%d - Can not process(>10): %s\n", s, str);
	    t++;
	}
	else
	    printf ("%s %s\n", strPYs + 1, strHZs);

      _next:
	;
    }

    fclose (fp);

    fprintf (stderr, "Total: %d  Error: %d\n", s, t);

    return 0;
}
