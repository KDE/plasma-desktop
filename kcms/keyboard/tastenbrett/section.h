/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SECTION_H
#define SECTION_H

#include "xkbobject.h"

class Section : public XkbObject
{
    Q_OBJECT

#define S_P(type, name)                                                                                                                                        \
private:                                                                                                                                                       \
    Q_PROPERTY(type name READ auto_prop_##name CONSTANT)                                                                                                       \
public:                                                                                                                                                        \
    type auto_prop_##name() const                                                                                                                              \
    {                                                                                                                                                          \
        return section->name;                                                                                                                                  \
    }

    S_P(unsigned char, priority)
    S_P(short, top)
    S_P(short, left)
    S_P(unsigned short, width)
    S_P(unsigned short, height)
    S_P(short, angle)

    Q_PROPERTY(QList<QObject *> rows MEMBER rows CONSTANT)
    Q_PROPERTY(QList<QObject *> doodads MEMBER doodads CONSTANT)
public:
    Section(XkbSectionPtr section_, XkbDescPtr xkb_, QObject *parent = nullptr);

    XkbSectionPtr section = nullptr;
    QList<QObject *> rows;
    QList<QObject *> doodads;
};

#endif // SECTION_H
