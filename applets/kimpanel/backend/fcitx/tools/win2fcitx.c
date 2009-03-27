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
 * 将windows输入法管理器生成的文本文件转换为小企鹅输入法的词库
 * windows生成的文本文件格式如下：
 *  汉字 编码 编码 ...
 * 汉字和编码间可能没有空格，多个编码间是空格 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main (int argc, char *argv[])
{
    FILE           *fp;
    char            str[100];
    char            strHZ[50];
    char            strCode[50];
    char           *pstr;
    int             i, s;

    if (argc != 2) {
	fprintf (stderr, "Usage: %s <WIN Phrase File>\n", argv[0]);
	exit (-1);
    }

    fp = fopen (argv[1], "rt");
    if (!fp) {
	fprintf (stderr, "Can't open file %s\n", argv[1]);
	exit (1);
    }

    s = 0;
    for (;;) {
	if (!fgets (str, 100, fp))
	    break;

	i = strlen (str) - 1;
	while ((i >= 0) && (str[i] == ' ' || str[i] == '\n' || str[i] == '\r'))
	    str[i--] = '\0';

	pstr = str;
	if (*pstr == ' ')
	    pstr++;

	i = 0;
	while (*pstr) {
	    if (!isalpha (*pstr)) {	//是汉字
		strHZ[i++] = *pstr++;
		strHZ[i] = *pstr;
	    }
	    else {		//汉字结束
		strHZ[i] = '\0';
		break;
	    }
	    pstr++;
	    i++;
	}

	if (*pstr == ' ')
	    pstr++;
	i = 0;
	for (;;) {
	    if (*pstr != ' ' && *pstr != '\0')
		strCode[i++] = *pstr++;
	    else {
		s++;
		strCode[i] = '\0';
		printf ("%s %s\n", strCode, strHZ);

		if (!*pstr)
		    break;

		i = 0;
		pstr++;
	    }
	}
    }

    fclose (fp);

    fprintf (stderr, "Total: %d\n", s);

    return 0;
}
