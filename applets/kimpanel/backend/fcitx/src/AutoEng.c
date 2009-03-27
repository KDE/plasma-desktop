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
#include "AutoEng.h"
#include "tools.h"

#include <limits.h>

AUTO_ENG       *AutoEng = (AUTO_ENG *) NULL;
int             iAutoEng;

/**
 * 从 ~/.fcitx/AutoEng.dat(如果不存在，
 * 则从 /usr/local/share/fcitx/data/AutoEng.dat）
 * 读取需要自动转换到英文输入状态的情况的数据。
 *
 * @param void
 *
 * @return void
 */

void LoadAutoEng (void)
{
    FILE           *fp;
    char            strPath[PATH_MAX];

    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/");
    strcat (strPath, "AutoEng.dat");

    if (access (strPath, 0)) {
	strcpy (strPath, PKGDATADIR "/data/");
	strcat (strPath, "AutoEng.dat");
    }

    fp = fopen (strPath, "rt");
    if (!fp)
	return;

    iAutoEng = CalculateRecordNumber (fp);
    AutoEng = (AUTO_ENG *) malloc (sizeof (AUTO_ENG) * iAutoEng);

    iAutoEng = 0;
    while  (!feof(fp)) {
	fscanf (fp, "%s\n", strPath);
	strcpy (AutoEng[iAutoEng++].str, strPath);
    }

    fclose (fp);
}

/**
 * 释放相关资源
 *
 * @param void
 *
 * @return void
 */
void FreeAutoEng (void)
{
    if (AutoEng)
	free (AutoEng);

    iAutoEng = 0;
    AutoEng = (AUTO_ENG *) NULL;
}


/**
 * @brief 判断是否需要自动转到英文输入状态
 * @exception <exception-object> <exception description>
 * @return Ture则是？ False则是？
 * @note <text>
 * @remarks <remark text>
 * [@deprecated <description>]
 * [@since when(time or version)]
 * [@see references{,references}]
 */
Bool SwitchToEng (char *str)
{
    int             i;

    for (i = 0; i < iAutoEng; i++) {
	if (!strcmp (str, AutoEng[i].str))
	    return True;
    }

    return False;
}
