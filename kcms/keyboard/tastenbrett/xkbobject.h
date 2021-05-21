/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef XKBOBJECT_H
#define XKBOBJECT_H

#include <QObject>

#include <X11/XKBlib.h>
#include <X11/extensions/XKBgeom.h>
#include <X11/extensions/XKBrules.h>
#include <X11/extensions/XKBstr.h>

// undef generic x stuff. should use kwindowsystem's fixx11h really
#undef Status
#undef None
#undef Bool
#undef CursorShape
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef Expose
#undef Unsorted

class XkbObject : public QObject
{
protected:
    XkbObject(XkbDescPtr xkb_, QObject *parent = nullptr);

    XkbDescPtr xkb = nullptr;
};

#endif // XKBOBJECT_H
