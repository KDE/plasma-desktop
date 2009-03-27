/***************************************************************************
 *   Copyright (C) 2002~2005 by Yuking                                     *
 *   yuking_net@sohu.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../src/internalVersion.c"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
    
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
    
#define MAX_CODE_LENGTH 30
    typedef struct _RULE_RULE {
    unsigned char   iFlag;	// 1 --> 正序   0 --> 逆序
    unsigned char   iWhich;	//第几个字
    unsigned char   iIndex;	//第几个编码
} RULE_RULE;
typedef struct _RULE {
    unsigned char   iWords;	//多少个字
    unsigned char   iFlag;	//1 --> 大于等于iWords  0 --> 等于iWords
    RULE_RULE      *rule;
} RULE;
int main (int argc, char *argv[])
{
    char            strCode[100];
    char            strHZ[100];
    FILE           *fpDict;
    unsigned int    i = 0;
    unsigned int    iTemp;
    unsigned int    j;
    unsigned char   iLen;
    unsigned char   iRule;
    unsigned char   iPYLen;
    char            iVersion = 0;

    if (argc != 2) {
	printf ("\nUsage: mb2txt <Source File>\n\n");
	exit (1);
    }

    fpDict = fopen (argv[1], "rb");
    if (!fpDict) {
	printf ("\nCan not read source file!\n\n");
	exit (2);
    }

    //先读取码表的信息
    fread (&iTemp, sizeof (unsigned int), 1, fpDict);
    if (iTemp == 0) {
	fread (&iVersion, sizeof (char), 1, fpDict);
	printf (";fcitx 版本 0x%02x 码表文件\n", iVersion);
	fread (&iTemp, sizeof (unsigned int), 1, fpDict);
    }
    else
	printf (";fcitx 版本 0x02 码表文件\n");

    fread (strCode, sizeof (char), iTemp + 1, fpDict);
    printf ("键码=%s\n", strCode);
    fread (&iLen, sizeof (unsigned char), 1, fpDict);
    printf ("码长=%d\n", iLen);

    if (iVersion) {
	fread (&iPYLen, sizeof (unsigned char), 1, fpDict);
	if (iPYLen) {
	    printf ("拼音=@\n");
	    printf ("拼音长度=%d\n", iPYLen);
	}
    }

    fread (&iTemp, sizeof (unsigned int), 1, fpDict);
    fread (strCode, sizeof (char), iTemp + 1, fpDict);
    if (iTemp)
	printf ("规避字符=%s\n", strCode);

    fread (&iRule, sizeof (unsigned char), 1, fpDict);
    if (iRule) {		//表示有组词规则
	printf ("[组词规则]\n");
	for (i = 0; i < iLen - 1; i++) {
	    fread (&iRule, sizeof (unsigned char), 1, fpDict);
	    printf ("%c", (iRule) ? 'a' : 'e');
	    fread (&iRule, sizeof (unsigned char), 1, fpDict);
	    printf ("%d=", iRule);
	    for (iTemp = 0; iTemp < iLen; iTemp++) {
		fread (&iRule, sizeof (unsigned char), 1, fpDict);
		printf ("%c", (iRule) ? 'p' : 'n');
		fread (&iRule, sizeof (unsigned char), 1, fpDict);
		printf ("%d", iRule);
		fread (&iRule, sizeof (unsigned char), 1, fpDict);
		printf ("%d", iRule);
		if (iTemp != (iLen - 1))
		    printf ("+");
	    }
	    printf ("\n");
	}
    }

    printf ("[数据]\n");
    fread (&j, sizeof (unsigned int), 1, fpDict);

    if (iVersion)
	iLen = iPYLen;

    for (i = 0; i < j; i++) {
	fread (strCode, sizeof (char), iLen + 1, fpDict);
	fread (&iTemp, sizeof (unsigned int), 1, fpDict);
	fread (strHZ, sizeof (unsigned char), iTemp, fpDict);

	if (iVersion) {
	    fread (&iRule, sizeof (unsigned char), 1, fpDict);
	    if (iRule)
		printf ("@%s %s\n", strCode, strHZ);
	    else
		printf ("%s %s\n", strCode, strHZ);
	}
	else
	    printf ("%s %s\n", strCode, strHZ);

	fread (&iTemp, sizeof (unsigned int), 1, fpDict);
	fread (&iTemp, sizeof (unsigned int), 1, fpDict);
    }

    fclose (fpDict);

    return 0;
}
