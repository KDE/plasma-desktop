#ifndef __CREATE_PARENT_H__
#define __CREATE_PARENT_H__

/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QLatin1String>
#include <QX11Info>
#include <X11/Xlib.h>

//
// *Very* hacky way to get some KDE dialogs to appear to be transient
// for 'xid'
//
// Create's a QWidget with size 0/0 and no border, makes this transient
// for xid, and all other widgets can use this as their parent...
static QWidget * createParent(int xid)
{
    if(!xid)
        return nullptr;

    QWidget *parent=new QWidget(nullptr, Qt::FramelessWindowHint);

    parent->resize(1, 1);
    parent->show();

    XWindowAttributes attr;
    int               rx, ry;
    Window            junkwin;

    XSetTransientForHint(QX11Info::display(), parent->winId(), xid);
    if(XGetWindowAttributes(QX11Info::display(), xid, &attr))
    {
        XTranslateCoordinates(QX11Info::display(), xid, attr.root,
                              -attr.border_width, -16,
                              &rx, &ry, &junkwin);

        rx=(rx+(attr.width/2));
        if(rx<0)
            rx=0;
        ry=(ry+(attr.height/2));
        if(ry<0)
            ry=0;
        parent->move(rx, ry);
    }
    parent->setWindowOpacity(0);
    parent->setWindowTitle(QLatin1String("KFI"));

    return parent;
}

#endif
