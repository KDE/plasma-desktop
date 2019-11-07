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

#include "row.h"

#include "key.h"

Row::Row(XkbRowPtr row_, XkbDescPtr xkb_, QObject *parent)
    : XkbObject(xkb_, parent)
    , row(row_)
    , orientation(row->vertical ? Qt::Vertical : Qt::Horizontal)
    , bounds(QRect(row->bounds.x1, row->bounds.y1,
                   row->bounds.x2, row->bounds.y2))
{
    for (int i = 0; i < row->num_keys; ++i) {
        keys.push_back(new Key(row->keys + i, xkb, this));
    }
}
