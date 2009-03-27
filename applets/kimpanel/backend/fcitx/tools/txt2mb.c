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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "../src/internalVersion.c"

typedef int     Bool;

#define MAX_CODE_LENGTH 30

#define True	1
#define False	0

char            strInputCode[100] = "\0";
char            strIgnoreChars[100] = "\0";
char            cPinyinKey = '\0';

typedef struct _RECORD {
    char           *strCode;
    char           *strHZ;
    INT8            bPinyin;
    struct _RECORD *next;
    struct _RECORD *prev;
    unsigned int    iHit;
    unsigned int    iIndex;
} RECORD;

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

Bool IsValidCode (char cChar)
{
    char           *p;

    p = strInputCode;
    while (*p) {
	if (cChar == *p)
	    return True;
	p++;
    }

    p = strIgnoreChars;
    while (*p) {
	if (cChar == *p)
	    return True;
	p++;
    }

    if (cChar == cPinyinKey)
	return True;

    return False;
}

int main (int argc, char *argv[])
{
    char            strCode[100];
    char            strHZ[100];
    char           *p;
    FILE           *fpDict, *fpNew;
    RECORD         *temp, *head, *newRec, *current;
    unsigned int    s = 0;
    int             i;
    unsigned int    iTemp;
    char           *pstr = 0;
    char            strTemp[10];
    unsigned char   bRule;
    RULE           *rule = NULL;
    unsigned int    l;

    unsigned char   iCodeLength = 0;
    unsigned char   iPYCodeLength = 0;

    Bool            bPY;

    if (argc != 3) {
	printf ("\nUsage: txt2mb <Source File> <IM File>\n\n");
	exit (1);
    }

    fpDict = fopen (argv[1], "rt");
    if (!fpDict) {
	printf ("\nCan not read source file!\n\n");
	exit (2);
    }

    head = (RECORD *) malloc (sizeof (RECORD));
    head->next = head;
    head->prev = head;
    current = head;

    bRule = 0;
    l = 0;
    for (;;) {
	l++;
	if (!fgets (strCode, 100, fpDict))
	    break;

	i = strlen (strCode) - 1;
	while ((i >= 0) && (strCode[i] == ' ' || strCode[i] == '\n' || strCode[i] == '\r'))
	    strCode[i--] = '\0';

	pstr = strCode;
	if (*pstr == ' ')
	    pstr++;
	if (pstr[0] == '#')
	    continue;

	if (strstr (pstr, "键码=")) {
	    pstr += 5;
	    strcpy (strInputCode, pstr);
	}
	else if (strstr (pstr, "码长=")) {
	    pstr += 5;
	    iCodeLength = atoi (pstr);
	}
	else if (strstr (pstr, "规避字符=")) {
	    pstr += 9;
	    strcpy (strIgnoreChars, pstr);
	}
	else if (strstr (pstr, "拼音=")) {
	    pstr += 5;
	    while (*pstr == ' ')
		pstr++;
	    cPinyinKey = *pstr;
	}
	else if (strstr (pstr, "拼音长度=")) {
	    pstr += 9;
	    iPYCodeLength = atoi (pstr);
	}

	else if (strstr (pstr, "[数据]"))
	    break;
	else if (strstr (pstr, "[组词规则]")) {
	    bRule = 1;
	    break;
	}
    }

    if (iCodeLength <= 0 || !strInputCode[0]) {
	printf ("Source File Format Error!\n");
	exit (1);
    }

    if (bRule) {
	/*
	 * 组词规则数应该比键码长度小1
	 */
	rule = (RULE *) malloc (sizeof (RULE) * (iCodeLength - 1));

	for (iTemp = 0; iTemp < (iCodeLength - 1); iTemp++) {
	    l++;
	    if (!fgets (strCode, 100, fpDict))
		break;

	    rule[iTemp].rule = (RULE_RULE *) malloc (sizeof (RULE_RULE) * iCodeLength);

	    i = strlen (strCode) - 1;
	    while ((i >= 0) && (strCode[i] == ' ' || strCode[i] == '\n' || strCode[i] == '\r'))
		strCode[i--] = '\0';

	    pstr = strCode;
	    if (*pstr == ' ')
		pstr++;
	    if (pstr[0] == '#')
		continue;

	    if (strstr (pstr, "[数据]"))
		break;

	    switch (*pstr) {
	    case 'e':
	    case 'E':
		rule[iTemp].iFlag = 0;
		break;
	    case 'a':
	    case 'A':
		rule[iTemp].iFlag = 1;
		break;
	    default:
		printf ("2   Phrase rules are not suitable!\n");
		printf ("\t\t%s\n", strCode);
		exit (1);
	    }
	    pstr++;
	    p = pstr;
	    while (*p && *p != '=')
		p++;
	    if (!(*p)) {
		printf ("3   Phrase rules are not suitable!\n");
		printf ("\t\t%s\n", strCode);
		exit (1);
	    }
	    strncpy (strTemp, pstr, p - pstr);
	    strTemp[p - pstr] = '\0';
	    rule[iTemp].iWords = atoi (strTemp);

	    p++;

	    for (i = 0; i < iCodeLength; i++) {
		while (*p == ' ')
		    p++;

		switch (*p) {
		case 'p':
		case 'P':
		    rule[iTemp].rule[i].iFlag = 1;
		    break;
		case 'n':
		case 'N':
		    rule[iTemp].rule[i].iFlag = 0;
		    break;
		default:
		    printf ("4   Phrase rules are not suitable!\n");
		    printf ("\t\t%s\n", strCode);
		    exit (1);
		}

		p++;

		rule[iTemp].rule[i].iWhich = *p++ - '0';
		rule[iTemp].rule[i].iIndex = *p++ - '0';

		while (*p == ' ')
		    p++;
		if (i != (iCodeLength - 1)) {
		    if (*p != '+') {
			printf ("5   Phrase rules are not suitable!\n");
			printf ("\t\t%s  %d\n", strCode, iCodeLength);
			exit (1);
		    }

		    p++;
		}
	    }
	}

	if (iTemp != iCodeLength - 1) {
	    printf ("6  Phrase rules are not suitable!\n");
	    exit (1);
	}

	for (iTemp = 0; iTemp < (iCodeLength - 1); iTemp++) {
	    l++;
	    if (!fgets (strCode, 100, fpDict))
		break;

	    i = strlen (strCode) - 1;
	    while ((i >= 0) && (strCode[i] == ' ' || strCode[i] == '\n' || strCode[i] == '\r'))
		strCode[i--] = '\0';

	    pstr = strCode;
	    if (*pstr == ' ')
		pstr++;
	    if (pstr[0] == '#')
		continue;

	    if (strstr (pstr, "[数据]"))
		break;
	}
    }

    if (iPYCodeLength < iCodeLength)
	iPYCodeLength = iCodeLength;

    if (!strstr (pstr, "[数据]")) {
	printf ("Source File Format Error!\n");
	exit (1);
    }

    for (;;) {
	l++;
	if (EOF == fscanf (fpDict, "%s %s\n", strCode, strHZ))
	    break;

	if (!IsValidCode (strCode[0])) {
	    printf ("Invalid Format: Line-%d  %s %s\n", l, strCode, strHZ);

	    exit (1);
	}

	if (((strCode[0] != cPinyinKey) && (strlen (strCode) > iCodeLength)) || ((strCode[0] == cPinyinKey) && (strlen (strCode) > (iPYCodeLength + 1))))
	    continue;
	if (strlen (strHZ) > 20)	//最长词组长度为10个汉字
	    continue;

	bPY = False;
	if (strCode[0] == cPinyinKey) {
	    strcpy (strCode, strCode + 1);
	    bPY = True;
	}

	//查找是否重复
	temp = current;
	if (temp != head) {
	    if (strcmp (temp->strCode, strCode) >= 0) {
		while (temp != head && strcmp (temp->strCode, strCode) >= 0) {
		    if (!strcmp (temp->strHZ, strHZ) && !strcmp (temp->strCode, strCode)) {
			printf ("Delete:  %s %s\n", strCode, strHZ);
			goto _next;
		    }
		    temp = temp->prev;
		}

		if (temp == head)
		    temp = temp->next;

		while (temp != head && strcmp (temp->strCode, strCode) <= 0)
		    temp = temp->next;
	    }
	    else {
		while (temp != head && strcmp (temp->strCode, strCode) <= 0) {
		    if (!strcmp (temp->strHZ, strHZ) && !strcmp (temp->strCode, strCode)) {
			printf ("Delete:  %s %s\n", strCode, strHZ);
			goto _next;
		    }
		    temp = temp->next;
		}
	    }
	}

	//插在temp的前面
	newRec = (RECORD *) malloc (sizeof (RECORD));
	newRec->strCode = (char *) malloc (sizeof (char) * (iPYCodeLength + 1));
	newRec->strHZ = (char *) malloc (sizeof (char) * strlen (strHZ) + 1);
	strcpy (newRec->strCode, strCode);
	strcpy (newRec->strHZ, strHZ);
	newRec->bPinyin = bPY;
	newRec->iHit = 0;
	newRec->iIndex = 0;

	temp->prev->next = newRec;
	newRec->next = temp;
	newRec->prev = temp->prev;
	temp->prev = newRec;

	current = newRec;

	s++;
      _next:
	continue;
    }

    fclose (fpDict);

    printf ("\nReading %d records.\n\n", s);

    fpNew = fopen (argv[2], "wb");
    if (!fpNew) {
	printf ("\nCan not create target file!\n\n");
	exit (3);
    }

    //写入版本号--如果第一个字为0,表示后面那个字节为版本号
    iTemp = 0;
    fwrite (&iTemp, sizeof (unsigned int), 1, fpDict);
    fwrite (&iInternalVersion, sizeof (INT8), 1, fpDict);

    iTemp = (unsigned int) strlen (strInputCode);
    fwrite (&iTemp, sizeof (unsigned int), 1, fpNew);
    fwrite (strInputCode, sizeof (char), iTemp + 1, fpNew);
    fwrite (&iCodeLength, sizeof (unsigned char), 1, fpNew);
    fwrite (&iPYCodeLength, sizeof (unsigned char), 1, fpNew);
    iTemp = (unsigned int) strlen (strIgnoreChars);
    fwrite (&iTemp, sizeof (unsigned int), 1, fpNew);
    fwrite (strIgnoreChars, sizeof (char), iTemp + 1, fpNew);

    fwrite (&bRule, sizeof (unsigned char), 1, fpNew);
    if (bRule) {
	for (i = 0; i < iCodeLength - 1; i++) {
	    fwrite (&(rule[i].iFlag), sizeof (unsigned char), 1, fpNew);
	    fwrite (&(rule[i].iWords), sizeof (unsigned char), 1, fpNew);
	    for (iTemp = 0; iTemp < iCodeLength; iTemp++) {
		fwrite (&(rule[i].rule[iTemp].iFlag), sizeof (unsigned char), 1, fpNew);
		fwrite (&(rule[i].rule[iTemp].iWhich), sizeof (unsigned char), 1, fpNew);
		fwrite (&(rule[i].rule[iTemp].iIndex), sizeof (unsigned char), 1, fpNew);
	    }
	}
    }

    fwrite (&s, sizeof (unsigned int), 1, fpNew);
    current = head->next;

    while (current != head) {
	fwrite (current->strCode, sizeof (char), iPYCodeLength + 1, fpNew);
	s = strlen (current->strHZ) + 1;
	fwrite (&s, sizeof (unsigned int), 1, fpNew);
	fwrite (current->strHZ, sizeof (char), s, fpNew);
	fwrite (&(current->bPinyin), sizeof (char), 1, fpDict);
	fwrite (&(current->iHit), sizeof (unsigned int), 1, fpNew);
	fwrite (&(current->iIndex), sizeof (unsigned int), 1, fpNew);
	current = current->next;
    }
    fclose (fpNew);

    return 0;
}
