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

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "xkbobject.h"

class Geometry : public XkbObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QObject *> doodads MEMBER doodads CONSTANT)
    Q_PROPERTY(QList<QObject *> sections MEMBER sections CONSTANT)
    Q_PROPERTY(qreal widthMM MEMBER widthMM CONSTANT)
    Q_PROPERTY(qreal heightMM MEMBER heightMM CONSTANT)
public:
    Geometry(XkbGeometryPtr geom_, XkbDescPtr xkb_, QObject *parent = nullptr);

    XkbGeometryPtr geom = nullptr;

    QList<QObject *> doodads;
    QList<QObject *> sections;

    qreal widthMM = -1;
    qreal heightMM = -1;
};

#endif // GEOMETRY_H
