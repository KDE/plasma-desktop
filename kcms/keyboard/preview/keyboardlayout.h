/*
 *  Copyright (C) 2012 Shivam Makkar (amourphious1992@gmail.com)
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
 
#ifndef KEYBOARDLAYOUT_H
#define KEYBOARDLAYOUT_H

#include "keysymbols.h"
#include "keyaliases.h"

#include <QApplication>

class KeyboardLayout
{
    QString layoutName;
    Aliases alias;
    
public:
    KeyboardLayout();

    KeySymbols TLDE;
    KeySymbols BKSL;
    KeySymbols AE[12];
    KeySymbols AD[12];
    KeySymbols AC[11];
    KeySymbols AB[11];

    void generateLayout(QString a, const QString &cname);
    QString findSymbolBaseDir();
    void includeSymbol(QString a, const QString &cname);
    QString getLayoutName() const {
    	return layoutName;
    }
};

#endif // KEYBOARDLAYOUT_H
