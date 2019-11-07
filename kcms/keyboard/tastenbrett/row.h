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

#ifndef ROW_H
#define ROW_H

#include <QRect>

#include "xkbobject.h"

class Row : public XkbObject
{
    Q_OBJECT

#define R_P(type, name) \
private: \
    Q_PROPERTY(type name READ auto_prop_##name CONSTANT) \
public: \
    type auto_prop_##name () const { return row-> name ; }

R_P(short, top)
R_P(short, left)

Q_PROPERTY(Qt::Orientation orientation MEMBER orientation CONSTANT)
Q_PROPERTY(QList<QObject *> keys MEMBER keys CONSTANT)
Q_PROPERTY(QRect bounds MEMBER bounds CONSTANT)
public:
    Row(XkbRowPtr row_, XkbDescPtr xkb_, QObject *parent = nullptr);

    XkbRowPtr row = nullptr;
    Qt::Orientation orientation = Qt::Horizontal;
    QList<QObject *> keys;
    QRect bounds;
};

#endif // ROW_H
