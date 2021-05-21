/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef OUTLINE_H
#define OUTLINE_H

#include "xkbobject.h"

class Outline : public XkbObject
{
    Q_OBJECT
#define P(type, name)                                                                                                                                          \
private:                                                                                                                                                       \
    Q_PROPERTY(type name READ auto_prop_##name CONSTANT)                                                                                                       \
public:                                                                                                                                                        \
    type auto_prop_##name() const                                                                                                                              \
    {                                                                                                                                                          \
        return outline->name;                                                                                                                                  \
    }

    P(unsigned short, corner_radius)

    Q_PROPERTY(QVariantList points MEMBER points CONSTANT)

public:
    Outline(XkbOutlinePtr outline_, XkbDescPtr xkb_, QObject *parent = nullptr);

    XkbOutlinePtr outline = nullptr;
    QVariantList points;
};

#endif // OUTLINE_H
