/*
    Copyright 2019 Harald Sitter <sitter@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SECTION_H
#define SECTION_H

#include "xkbobject.h"

class Section : public XkbObject
{
    Q_OBJECT

#define S_P(type, name) \
private: \
    Q_PROPERTY(type name READ auto_prop_##name CONSTANT) \
public: \
    type auto_prop_##name () const { return section-> name ; }

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
