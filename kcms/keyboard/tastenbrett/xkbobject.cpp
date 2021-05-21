/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "xkbobject.h"

XkbObject::XkbObject(XkbDescPtr xkb_, QObject *parent)
    : QObject(parent)
    , xkb(xkb_)
{
    Q_ASSERT(xkb);
}
