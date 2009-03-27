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
#include "PYFA.h"

#include <stdio.h>

MHPY            MHPY_C[] = {	//韵母
    //{"an","ang"},
    {"CD", 0}
    ,
    //{"en","eng"},
    {"HI", 0}
    ,
    //{"ian","iang"},
    {"LM", 0}
    ,
    //{"in","ing"},
    {"PQ", 0}
    ,
    //{"u","ou"},
    {"VW", 0}
    ,
    //{"uan","uang"},
    {"Za", 0}
    ,

    {"\0", 0}
};

MHPY            MHPY_S[] = {	//声母
    //{"c","ch"},
    {"bc", 0}
    ,
    //{"f","h"},
    {"TV", 0}
    ,
    //{"l","n"},
    {"OQ", 0}
    ,
    //{"s","sh"},
    {"GH", 0}
    ,
    //{"z","zh"},
    {"AB", 0}
    ,
    //{"an","ang"}
    {"fg", 0}
    ,

    {"\0", 0}
};

//其中增加了那些不是标准的拼音，但模糊输入中需要使用的拼音组合
PYTABLE         PYTable[] = {
    {"zuo", NULL}
    ,
    {"zun", NULL}
    ,
    {"zui", NULL}
    ,
    {"zuang", &MHPY_C[5].bMode}
    ,
    {"zuang", &MHPY_S[4].bMode}
    ,
    {"zuan", NULL}
    ,
    {"zua", &MHPY_S[4].bMode}
    ,
    {"zu", NULL}
    ,
    {"zou", NULL}
    ,
    {"zong", NULL}
    ,
    {"zi", NULL}
    ,
    {"zhuo", NULL}
    ,
    {"zhun", NULL}
    ,
    {"zhui", NULL}
    ,
    {"zhuang", NULL}
    ,
    {"zhuan", NULL}
    ,
    {"zhuai", NULL}
    ,
    {"zhua", NULL}
    ,
    {"zhu", NULL}
    ,
    {"zhou", NULL}
    ,
    {"zhong", NULL}
    ,
    {"zhi", NULL}
    ,
    {"zheng", NULL}
    ,
    {"zhen", NULL}
    ,
    {"zhe", NULL}
    ,
    {"zhao", NULL}
    ,
    {"zhang", NULL}
    ,
    {"zhan", NULL}
    ,
    {"zhai", NULL}
    ,
    {"zha", NULL}
    ,
    {"zeng", NULL}
    ,
    {"zen", NULL}
    ,
    {"zei", NULL}
    ,
    {"ze", NULL}
    ,
    {"zao", NULL}
    ,
    {"zang", NULL}
    ,
    {"zan", NULL}
    ,
    {"zai", NULL}
    ,
    {"za", NULL}
    ,
    {"yun", NULL}
    ,
    {"yue", NULL}
    ,
    {"yuang", &MHPY_C[5].bMode}
    ,
    {"yuan", NULL}
    ,
    {"yu", NULL}
    ,
    {"you", NULL}
    ,
    {"yong", NULL}
    ,
    {"yo", NULL}
    ,
    {"ying", NULL}
    ,
    {"yin", NULL}
    ,
    {"yi", NULL}
    ,
    {"ye", NULL}
    ,
    {"yao", NULL}
    ,
    {"yang", NULL}
    ,
    {"yan", NULL}
    ,
    {"ya", NULL}
    ,
    {"xun", NULL}
    ,
    {"xue", NULL}
    ,
    {"xuang", &MHPY_C[5].bMode}
    ,
    {"xuan", NULL}
    ,
    {"xu", NULL}
    ,
    {"xou", &MHPY_C[4].bMode}
    ,
    {"xiu", NULL}
    ,
    {"xiong", NULL}
    ,
    {"xing", NULL}
    ,
    {"xin", NULL}
    ,
    {"xie", NULL}
    ,
    {"xiao", NULL}
    ,
    {"xiang", NULL}
    ,
    {"xian", NULL}
    ,
    {"xia", NULL}
    ,
    {"xi", NULL}
    ,
    {"wu", NULL}
    ,
    {"wo", NULL}
    ,
    {"weng", NULL}
    ,
    {"wen", NULL}
    ,
    {"wei", NULL}
    ,
    {"wang", NULL}
    ,
    {"wan", NULL}
    ,
    {"wai", NULL}
    ,
    {"wa", NULL}
    ,
    {"tuo", NULL}
    ,
    {"tun", NULL}
    ,
    {"tui", NULL}
    ,
    {"tuang", &MHPY_C[5].bMode}
    ,
    {"tuan", NULL}
    ,
    {"tu", NULL}
    ,
    {"tou", NULL}
    ,
    {"tong", NULL}
    ,
    {"ting", NULL}
    ,
    {"tin", &MHPY_C[3].bMode}
    ,
    {"tie", NULL}
    ,
    {"tiao", NULL}
    ,
    {"tiang", &MHPY_C[2].bMode}
    ,
    {"tian", NULL}
    ,
    {"ti", NULL}
    ,
    {"teng", NULL}
    ,
    {"ten", &MHPY_C[1].bMode}
    ,
    {"tei", NULL}
    ,
    {"te", NULL}
    ,
    {"tao", NULL}
    ,
    {"tang", NULL}
    ,
    {"tan", NULL}
    ,
    {"tai", NULL}
    ,
    {"ta", NULL}
    ,
    {"suo", NULL}
    ,
    {"sun", NULL}
    ,
    {"sui", NULL}
    ,
    {"suang", &MHPY_S[3].bMode}
    ,
    {"suang", &MHPY_C[5].bMode}
    ,
    {"suan", NULL}
    ,
    {"sua", &MHPY_S[3].bMode}
    ,
    {"su", NULL}
    ,
    {"sou", NULL}
    ,
    {"song", NULL}
    ,
    {"si", NULL}
    ,
    {"shuo", NULL}
    ,
    {"shun", NULL}
    ,
    {"shui", NULL}
    ,
    {"shuang", NULL}
    ,
    {"shuan", NULL}
    ,
    {"shuai", NULL}
    ,
    {"shua", NULL}
    ,
    {"shu", NULL}
    ,
    {"shou", NULL}
    ,
    {"shi", NULL}
    ,
    {"sheng", NULL}
    ,
    {"shen", NULL}
    ,
    {"shei", NULL}
    ,
    {"she", NULL}
    ,
    {"shao", NULL}
    ,
    {"shang", NULL}
    ,
    {"shan", NULL}
    ,
    {"shai", NULL}
    ,
    {"sha", NULL}
    ,
    {"seng", NULL}
    ,
    {"sen", NULL}
    ,
    {"se", NULL}
    ,
    {"sao", NULL}
    ,
    {"sang", NULL}
    ,
    {"san", NULL}
    ,
    {"sai", NULL}
    ,
    {"sa", NULL}
    ,
    {"ruo", NULL}
    ,
    {"run", NULL}
    ,
    {"rui", NULL}
    ,
    {"ruang", &MHPY_C[5].bMode}
    ,
    {"ruan", NULL}
    ,
    {"ru", NULL}
    ,
    {"rou", NULL}
    ,
    {"rong", NULL}
    ,
    {"ri", NULL}
    ,
    {"reng", NULL}
    ,
    {"ren", NULL}
    ,
    {"re", NULL}
    ,
    {"rao", NULL}
    ,
    {"rang", NULL}
    ,
    {"ran", NULL}
    ,
    {"qun", NULL}
    ,
    {"que", NULL}
    ,
    {"quang", &MHPY_C[5].bMode}
    ,
    {"quan", NULL}
    ,
    {"qu", NULL}
    ,
    {"qiu", NULL}
    ,
    {"qiong", NULL}
    ,
    {"qing", NULL}
    ,
    {"qin", NULL}
    ,
    {"qie", NULL}
    ,
    {"qiao", NULL}
    ,
    {"qiang", NULL}
    ,
    {"qian", NULL}
    ,
    {"qia", NULL}
    ,
    {"qi", NULL}
    ,
    {"pu", NULL}
    ,
    {"pou", NULL}
    ,
    {"po", NULL}
    ,
    {"ping", NULL}
    ,
    {"pin", NULL}
    ,
    {"pie", NULL}
    ,
    {"piao", NULL}
    ,
    {"piang", &MHPY_C[2].bMode}
    ,
    {"pian", NULL}
    ,
    {"pi", NULL}
    ,
    {"peng", NULL}
    ,
    {"pen", NULL}
    ,
    {"pei", NULL}
    ,
    {"pao", NULL}
    ,
    {"pang", NULL}
    ,
    {"pan", NULL}
    ,
    {"pai", NULL}
    ,
    {"pa", NULL}
    ,
    {"ou", NULL}
    ,
    {"o", NULL}
    ,
    {"nve", NULL}
    ,
    {"nv", NULL}
    ,
    {"nuo", NULL}
    ,
    {"nue", NULL}
    ,
    {"nuang", &MHPY_C[5].bMode}
    ,
    {"nuang", &MHPY_S[2].bMode}
    ,
    {"nuan", NULL}
    ,
    {"nu", NULL}
    ,
    {"nou", NULL}
    ,
    {"nong", NULL}
    ,
    {"niu", NULL}
    ,
    {"ning", NULL}
    ,
    {"nin", NULL}
    ,
    {"nie", NULL}
    ,
    {"niao", NULL}
    ,
    {"niang", NULL}
    ,
    {"nian", NULL}
    ,
    {"ni", NULL}
    ,
    {"ng", NULL}
    ,
    {"neng", NULL}
    ,
    {"nen", NULL}
    ,
    {"nei", NULL}
    ,
    {"ne", NULL}
    ,
    {"nao", NULL}
    ,
    {"nang", NULL}
    ,
    {"nan", NULL}
    ,
    {"nai", NULL}
    ,
    {"na", NULL}
    ,
    {"n", NULL}
    ,
    {"mu", NULL}
    ,
    {"mou", NULL}
    ,
    {"mo", NULL}
    ,
    {"miu", NULL}
    ,
    {"ming", NULL}
    ,
    {"min", NULL}
    ,
    {"mie", NULL}
    ,
    {"miao", NULL}
    ,
    {"miang", &MHPY_C[2].bMode}
    ,
    {"mian", NULL}
    ,
    {"mi", NULL}
    ,
    {"meng", NULL}
    ,
    {"men", NULL}
    ,
    {"mei", NULL}
    ,
    {"me", NULL}
    ,
    {"mao", NULL}
    ,
    {"mang", NULL}
    ,
    {"man", NULL}
    ,
    {"mai", NULL}
    ,
    {"ma", NULL}
    ,
    {"m", NULL}
    ,
    {"lve", NULL}
    ,
    {"lv", NULL}
    ,
    {"luo", NULL}
    ,
    {"lun", NULL}
    ,
    {"lue", NULL}
    ,
    {"luang", &MHPY_C[5].bMode}
    ,
    {"luang", &MHPY_S[2].bMode}
    ,
    {"luan", NULL}
    ,
    {"lu", NULL}
    ,
    {"lou", NULL}
    ,
    {"long", NULL}
    ,
    {"liu", NULL}
    ,
    {"ling", NULL}
    ,
    {"lin", NULL}
    ,
    {"lie", NULL}
    ,
    {"liao", NULL}
    ,
    {"liang", NULL}
    ,
    {"lian", NULL}
    ,
    {"lia", NULL}
    ,
    {"li", NULL}
    ,
    {"leng", NULL}
    ,
    {"lei", NULL}
    ,
    {"le", NULL}
    ,
    {"lao", NULL}
    ,
    {"lang", NULL}
    ,
    {"lan", NULL}
    ,
    {"lai", NULL}
    ,
    {"la", NULL}
    ,
    {"kuo", NULL}
    ,
    {"kun", NULL}
    ,
    {"kui", NULL}
    ,
    {"kuang", NULL}
    ,
    {"kuan", NULL}
    ,
    {"kuai", NULL}
    ,
    {"kua", NULL}
    ,
    {"ku", NULL}
    ,
    {"kou", NULL}
    ,
    {"kong", NULL}
    ,
    {"keng", NULL}
    ,
    {"ken", NULL}
    ,
    {"kei", NULL}
    ,
    {"ke", NULL}
    ,
    {"kao", NULL}
    ,
    {"kang", NULL}
    ,
    {"kan", NULL}
    ,
    {"kai", NULL}
    ,
    {"ka", NULL}
    ,
    {"jun", NULL}
    ,
    {"jue", NULL}
    ,
    {"juang", &MHPY_C[5].bMode}
    ,
    {"juan", NULL}
    ,
    {"ju", NULL}
    ,
    {"jiu", NULL}
    ,
    {"jiong", NULL}
    ,
    {"jing", NULL}
    ,
    {"jin", NULL}
    ,
    {"jie", NULL}
    ,
    {"jiao", NULL}
    ,
    {"jiang", NULL}
    ,
    {"jian", NULL}
    ,
    {"jia", NULL}
    ,
    {"ji", NULL}
    ,
    {"huo", NULL}
    ,
    {"hun", NULL}
    ,
    {"hui", NULL}
    ,
    {"huang", NULL}
    ,
    {"huan", NULL}
    ,
    {"huai", NULL}
    ,
    {"hua", NULL}
    ,
    {"hu", NULL}
    ,
    {"hou", NULL}
    ,
    {"hong", NULL}
    ,
    {"heng", NULL}
    ,
    {"hen", NULL}
    ,
    {"hei", NULL}
    ,
    {"he", NULL}
    ,
    {"hao", NULL}
    ,
    {"hang", NULL}
    ,
    {"han", NULL}
    ,
    {"hai", NULL}
    ,
    {"ha", NULL}
    ,
    {"guo", NULL}
    ,
    {"gun", NULL}
    ,
    {"gui", NULL}
    ,
    {"guang", NULL}
    ,
    {"guan", NULL}
    ,
    {"guai", NULL}
    ,
    {"gua", NULL}
    ,
    {"gu", NULL}
    ,
    {"gou", NULL}
    ,
    {"gong", NULL}
    ,
    {"geng", NULL}
    ,
    {"gen", NULL}
    ,
    {"gei", NULL}
    ,
    {"ge", NULL}
    ,
    {"gao", NULL}
    ,
    {"gang", NULL}
    ,
    {"gan", NULL}
    ,
    {"gai", NULL}
    ,
    {"ga", NULL}
    ,
    {"fu", NULL}
    ,
    {"fou", NULL}
    ,
    {"fo", NULL}
    ,
    {"feng", NULL}
    ,
    {"fen", NULL}
    ,
    {"fei", NULL}
    ,
    {"fang", NULL}
    ,
    {"fan", NULL}
    ,
    {"fa", NULL}
    ,
    {"er", NULL}
    ,
    {"eng", &MHPY_C[1].bMode}
    ,
    {"en", NULL}
    ,
    {"ei", NULL}
    ,
    {"e", NULL}
    ,
    {"duo", NULL}
    ,
    {"dun", NULL}
    ,
    {"dui", NULL}
    ,
    {"duang", &MHPY_C[5].bMode}
    ,
    {"duan", NULL}
    ,
    {"du", NULL}
    ,
    {"dou", NULL}
    ,
    {"dong", NULL}
    ,
    {"diu", NULL}
    ,
    {"ding", NULL}
    ,
    {"din", &MHPY_C[3].bMode}
    ,
    {"die", NULL}
    ,
    {"diao", NULL}
    ,
    {"diang", &MHPY_C[2].bMode}
    ,
    {"dian", NULL}
    ,
    {"dia", NULL}
    ,
    {"di", NULL}
    ,
    {"deng", NULL}
    ,
    {"den", NULL}
    ,
    {"dei", NULL}
    ,
    {"de", NULL}
    ,
    {"dao", NULL}
    ,
    {"dang", NULL}
    ,
    {"dan", NULL}
    ,
    {"dai", NULL}
    ,
    {"da", NULL}
    ,
    {"cuo", NULL}
    ,
    {"cun", NULL}
    ,
    {"cui", NULL}
    ,
    {"cuang", &MHPY_C[5].bMode}
    ,
    {"cuang", &MHPY_S[0].bMode}
    ,
    {"cuan", NULL}
    ,
    {"cu", NULL}
    ,
    {"cou", NULL}
    ,
    {"cong", NULL}
    ,
    {"ci", NULL}
    ,
    {"chuo", NULL}
    ,
    {"chun", NULL}
    ,
    {"chui", NULL}
    ,
    {"chuang", NULL}
    ,
    {"chuan", NULL}
    ,
    {"chuai", NULL}
    ,
    {"chu", NULL}
    ,
    {"chou", NULL}
    ,
    {"chong", NULL}
    ,
    {"chi", NULL}
    ,
    {"cheng", NULL}
    ,
    {"chen", NULL}
    ,
    {"che", NULL}
    ,
    {"chao", NULL}
    ,
    {"chang", NULL}
    ,
    {"chan", NULL}
    ,
    {"chai", NULL}
    ,
    {"cha", NULL}
    ,
    {"ceng", NULL}
    ,
    {"cen", NULL}
    ,
    {"ce", NULL}
    ,
    {"cao", NULL}
    ,
    {"cang", NULL}
    ,
    {"can", NULL}
    ,
    {"cai", NULL}
    ,
    {"ca", NULL}
    ,
    {"bu", NULL}
    ,
    {"bo", NULL}
    ,
    {"bing", NULL}
    ,
    {"bin", NULL}
    ,
    {"bie", NULL}
    ,
    {"biao", NULL}
    ,
    {"biang", &MHPY_C[2].bMode}
    ,
    {"bian", NULL}
    ,
    {"bi", NULL}
    ,
    {"beng", NULL}
    ,
    {"ben", NULL}
    ,
    {"bei", NULL}
    ,
    {"bao", NULL}
    ,
    {"bang", NULL}
    ,
    {"ban", NULL}
    ,
    {"bai", NULL}
    ,
    {"ba", NULL}
    ,
    {"ao", NULL}
    ,
    {"ang", NULL}
    ,
    {"an", NULL}
    ,
    {"ai", NULL}
    ,
    {"a", NULL}
    ,
    {"\0", NULL}
};

int GetMHIndex_C (char map)
{
    int             i;

    for (i = 0; MHPY_C[i].strMap[0]; i++) {
	if (map == MHPY_C[i].strMap[0] || map == MHPY_C[i].strMap[1]) {
	    if (MHPY_C[i].bMode)
		return i;
	    else
		return -1;
	}
    }
    return -1;
}

int GetMHIndex_S (char map, Bool bMode)
{
    int             i;

    for (i = 0; MHPY_S[i].strMap[0]; i++) {
	if (map == MHPY_S[i].strMap[0] || map == MHPY_S[i].strMap[1]) {
	    if (MHPY_S[i].bMode || bMode)
		return i;
	    else
		return -1;
	}
    }

    return -1;

}

Bool IsZ_C_S (char map)
{
    if (map=='c' || map=='H'|| map=='B')
	return True;
    return False;
}

