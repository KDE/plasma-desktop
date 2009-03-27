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
#include "punc.h"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "ime.h"
#include "tools.h"

ChnPunc        *chnPunc = (ChnPunc *) NULL;

extern int      iCodeInputCount;

/**
 * @brief 加载标点词典
 * @param void
 * @return void
 * @note 文件中数据的格式为： 对应的英文符号 中文标点 <中文标点>
 * 加载标点词典。标点词典定义了一组标点转换，如输入‘.’就直接转换成‘。’
 */
int LoadPuncDict (void)
{
    FILE           *fpDict;				// 词典文件指针
    int             iRecordNo;
    char            strText[11];
    char            strPath[PATH_MAX];	// 词典文件的全名（包含绝对路径）
    char           *pstr;				// 临时指针
    int             i;

    // 构造词典文件的全名
    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/");
    strcat (strPath, PUNC_DICT_FILENAME);

    // 如果该文件不存在，就使用安装目录下的文件
    if (access (strPath, 0)) {
	strcpy (strPath, PKGDATADIR "/data/");
	strcat (strPath, PUNC_DICT_FILENAME);
    }

    // 如果无论是用户目录还是安装目录下的词典文件都无法打开，就给出提示信息，并退出程序
    fpDict = fopen (strPath, "rt");

    if (!fpDict) {
	printf ("Can't open Chinese punc file: %s\n", strPath);
	return False;
    }

    /* 计算词典里面有多少的数据
     * 这个函数非常简单，就是计算该文件有多少行（包含空行）。
     * 因为空行，在下面会略去，所以，这儿存在内存的浪费现象。
     * 没有一个空行就是浪费sizeof (ChnPunc)字节内存*/
    iRecordNo = CalculateRecordNumber (fpDict);
    // 申请空间，用来存放这些数据。这儿没有检查是否申请到内存，严格说有小隐患
    // chnPunc是一个全局变量
    chnPunc = (ChnPunc *) malloc (sizeof (ChnPunc) * (iRecordNo + 1));

    iRecordNo = 0;

    // 下面这个循环，就是一行一行的读入词典文件的数据。并将其放入到chnPunc里面去。
    for (;;) {
	if (!fgets (strText, 10, fpDict))
	    break;
	i = strlen (strText) - 1;

	// 先找到最后一个字符
	while ((strText[i] == '\n') || (strText[i] == ' ')) {
	    if (!i)
		break;
	    i--;
	}

	// 如果找到，进行出入。当是空行时，肯定找不到。所以，也就略过了空行的处理
	if (i) {
	    strText[i + 1] = '\0';				// 在字符串的最后加个封口
	    pstr = strText;						// 将pstr指向第一个非空字符
	    while (*pstr == ' ')
		pstr++;
	    chnPunc[iRecordNo].ASCII = *pstr++;	// 这个就是中文符号所对应的ASCII码值
	    while (*pstr == ' ')				// 然后，将pstr指向下一个非空字符
		pstr++;

	    chnPunc[iRecordNo].iCount = 0;		// 该符号有几个转化，比如英文"就可以转换成“和”
	    chnPunc[iRecordNo].iWhich = 0;		// 标示该符号的输入状态，即处于第几个转换。如"，iWhich标示是转换成“还是”
	    // 依次将该ASCII码所对应的符号放入到结构中
	    while (*pstr) {
		i = 0;
		// 因为中文符号都是双字节的，所以，要一直往后读，知道空格或者字符串的末尾
		while (*pstr != ' ' && *pstr) {
		    chnPunc[iRecordNo].strChnPunc[chnPunc[iRecordNo].iCount][i] = *pstr;
		    i++;
		    pstr++;
		}

		// 每个中文符号用'\0'隔开
		chnPunc[iRecordNo].strChnPunc[chnPunc[iRecordNo].iCount][i] = '\0';
		while (*pstr == ' ')
		    pstr++;
		chnPunc[iRecordNo].iCount++;
	    }

	    iRecordNo++;
	}
    }

    chnPunc[iRecordNo].ASCII = '\0';
    fclose (fpDict);

    return True;
}

void FreePunc (void)
{
    if (!chnPunc)
	return;

    free (chnPunc);
    chnPunc = (ChnPunc *) NULL;
}

/*
 * 根据字符得到相应的标点符号
 * 如果该字符不在标点符号集中，则返回NULL
 */
char           *GetPunc (int iKey)
{
    int             iIndex = 0;
    char           *pPunc;

    if (!chnPunc)
	return (char *) NULL;

    while (chnPunc[iIndex].ASCII) {
	if (chnPunc[iIndex].ASCII == iKey) {
	    pPunc = chnPunc[iIndex].strChnPunc[chnPunc[iIndex].iWhich];
	    chnPunc[iIndex].iWhich++;
	    if (chnPunc[iIndex].iWhich >= chnPunc[iIndex].iCount)
		chnPunc[iIndex].iWhich = 0;
	    return pPunc;
	}
	iIndex++;
    }

    return (char *) NULL;
}
