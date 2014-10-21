/*
 *  Copyright (C) 2013 Shivam Makkar (amourphious1992@gmail.com)
 *
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include "keyboardlayout.h"

#include <QDebug>
#include <QString>
#include <QList>


KbKey::KbKey()
{
    symbolCount = 0;
    symbols << QString();
}


void KbKey::setKeyName(QString n)
{
    keyName = n;
}


void KbKey::addSymbol(QString n, int i)
{
    if (!symbols.contains(n)) {
        symbols[i] = n;
        symbolCount++;
        symbols << QString();
    }
}


QString KbKey::getSymbol(int i)
{
    if (i < symbolCount) {
        return symbols[i];
    } else {
        return QString();
    }
}


void KbKey::display()
{
    qCDebug(KEYBOARD_PREVIEW) << keyName << " : ";
    for (int i = 0; i < symbolCount; i++) {
        qCDebug(KEYBOARD_PREVIEW) << "\t" << symbols[i];
    }
}


KbLayout::KbLayout()
{
    keyCount = 0;
    includeCount = 0;
    level = 4;
    keyList << KbKey();
    include << QString();
    parsedSymbol = true;
}


void KbLayout::setName(QString n)
{
    name = n;
}


void KbLayout::addInclude(QString n)
{
    if (!include.contains(n)) {
        include[includeCount] = n;
        includeCount++;
        include << QString();
    }
}


void KbLayout :: addKey()
{
    keyCount++;
    keyList << KbKey();
}


QString KbLayout :: getInclude(int i)
{
    if (i < includeCount) {
        return include[i];
    } else {
        return QString();
    }
}


int KbLayout :: findKey(QString n)
{
    for (int i = 0 ; i < keyCount ; i++) {
        if (keyList[i].keyName == n) {
            return i;
        }
    }
    return -1;
}


void KbLayout::display()
{
//    qCDebug(KEYBOARD_PREVIEW) << name << "\n";
//    for(int i = 0; i<includeCount; i++){
//        qCDebug(KEYBOARD_PREVIEW) << include[i];
//    }
    for (int i = 0 ; i < keyCount; i++) {
        keyList[i].display();
    }
}
