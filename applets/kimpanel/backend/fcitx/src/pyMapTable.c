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
#include "pyMapTable.h"

ConsonantMap    consonantMapTable[] = {
    {"a", 'A'}
    ,
    {"ai", 'B'}
    ,
    {"an", 'C'}
    ,
    {"ang", 'D'}
    ,
    {"ao", 'E'}
    ,

    {"e", 'F'}
    ,
    {"ei", 'G'}
    ,
    {"en", 'H'}
    ,
    {"eng", 'I'}
    ,

    {"i", 'J'}
    ,
    {"ia", 'K'}
    ,
    {"ian", 'L'}
    ,
    {"iang", 'M'}
    ,
    {"iao", 'N'}
    ,
    {"ie", 'O'}
    ,
    {"in", 'P'}
    ,
    {"ing", 'Q'}
    ,
    {"iong", 'R'}
    ,
    {"iu", 'S'}
    ,

    {"o", 'T'}
    ,
    {"ong", 'U'}
    ,
    {"ou", 'V'}
    ,

    {"u", 'W'}
    ,
    {"ua", 'X'}
    ,
    {"uai", 'Y'}
    ,
    {"uan", 'Z'}
    ,
    {"uang", 'a'}
    ,
    {"ue", 'b'}
    ,
    {"ui", 'c'}
    ,
    {"un", 'd'}
    ,
    {"uo", 'e'}
    ,

    {"v", 'f'}
    ,
    {"ve", 'g'}
    ,
/*	{"ve",'A'},
	{"v", 'B'},
	
	{"uo", 'C'},
	{"un", 'D'},
	{"ui", 'E'},
	{"ue", 'F'},
	{"uang", 'G'},
	{"uan", 'H'},
	{"uai", 'I'},
	{"ua", 'J'},
	{"u", 'K'},

	{"ou", 'L'},
	{"ong", 'M'},
	{"o", 'N'},

	{"iu", 'O'},
	{"iong", 'P'},
	{"ing", 'Q'},
	{"in", 'R'},
	{"ie", 'S'},
	{"iao", 'T'},
	{"iang", 'U'},
	{"ian", 'V'},
	{"ia", 'W'},
	{"i", 'X'},
	
	{"eng", 'Y'},
	{"en", 'Z'},
	{"ei", 'a'},
	{"e", 'b'},
	
	{"ao", 'c'},
	{"ang", 'd'},
	{"an", 'e'},
	{"ai", 'f'},
	{"a", 'g'},
  */
    {"\0", '\0'}
    ,
};

/*
 * ÉùÄ¸
 */
SyllabaryMap    syllabaryMapTable[] = {
    /*{"b", 'A'},
       {"c", 'B'},
       {"ch", 'C'},
       {"d", 'D'},
       {"f", 'E'},
       {"g", 'F'},
       {"h", 'G'},
       {"j", 'F'},
       {"k", 'I'},
       {"l", 'J'},
       {"m", 'K'},
       {"n", 'L'},
       {"p", 'M'},
       {"q", 'N'},
       {"r", 'O'},
       {"s", 'P'},
       {"sh", 'Q'},
       {"t", 'R'},
       {"w", 'S'},
       {"x", 'T'},
       {"y", 'U'},
       {"z", 'V'},
       {"zh", 'W'}, */

    {"zh", 'A'}
    ,
    {"z", 'B'}
    ,
    {"y", 'C'}
    ,
    {"x", 'D'}
    ,
    {"w", 'E'}
    ,
    {"t", 'F'}
    ,
    {"sh", 'G'}
    ,
    {"s", 'H'}
    ,
    {"r", 'I'}
    ,
    {"q", 'J'}
    ,
    {"p", 'K'}
    ,
    {"ou", 'L'}
    ,
    {"o", 'M'}
    ,
    {"ng", 'N'}
    ,
    {"n", 'O'}
    ,
    {"m", 'P'}
    ,
    {"l", 'Q'}
    ,
    {"k", 'R'}
    ,
    {"j", 'S'}
    ,
    {"h", 'T'}
    ,
    {"g", 'U'}
    ,
    {"f", 'V'}
    ,
    {"er", 'W'}
    ,
    {"en", 'X'}
    ,
    {"ei", 'Y'}
    ,
    {"e", 'Z'}
    ,
    {"d", 'a'}
    ,
    {"ch", 'b'}
    ,
    {"c", 'c'}
    ,
    {"b", 'd'}
    ,
    {"ao", 'e'}
    ,
    {"ang", 'f'}
    ,
    {"an", 'g'}
    ,
    {"ai", 'h'}
    ,
    {"a", 'i'}
    ,

    {"\0", '\0'}
};
