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

#include "keysymhelper.h"
#include "keysym2ucs.h"

#include <QtCore/QString>
#include <QtCore/QChar>

#include <KDebug>

#include <X11/XKBlib.h>


KeySymHelper::KeySymHelper()
{
    nill = 0;
}

QString KeySymHelper::getKeySymbol(const QString& opton)
{
    if( keySymbolMap.contains(opton) )
        return keySymbolMap[opton];

    const char* str = opton.toAscii().data();
    
#if 0
    //TODO: figure out how to use this so we don't need our own symkey2ucs mapping
    int res = Xutf8LookupString(XIC ic, XKeyPressedEvent *event, char *buffer_return, int bytes_buffer, KeySym *keysym_return, Status *status_return);

#else

    KeySym keysym = XStringToKeysym(str);
    long ucs = keysym2ucs(keysym);

//    if( ucs == -1 && (keysym >= 0xFE50 && keysym <= 0xFE5F) ) {
//        ucs = 0x0300 + (keysym & 0x000F);
//        kWarning() << "Got dead symbol" << QString("0x%1").arg(keysym, 0, 16) << "named" << opton << "will use" << QString("0x%1").arg(ucs, 0, 16) << "as UCS";
//    }

    if( ucs == -1 ) {
        nill++;
        kWarning() << "No mapping from keysym:" << QString("0x%1").arg(keysym, 0, 16) << "named:" << opton << "to UCS";
    }
    
    QString ucsStr = QString(QChar((int)ucs));
    
    // Combining Diacritical Marks
    if( ucs >= 0x0300 && ucs <= 0x036F ) {
        ucsStr = " " + ucsStr + " ";
    }

//    kWarning() << "--" << opton << "keysym: " << keysym << QString("0x%1").arg(keysym, 0, 16) << "keysym2string" << XKeysymToString(keysym)
//         << "---" << QString("0x%1").arg(ucs, 0, 16) << ucsStr;

    keySymbolMap[opton] = ucsStr;

    return ucsStr;

#endif

}

