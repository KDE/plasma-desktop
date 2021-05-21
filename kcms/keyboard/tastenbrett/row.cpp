/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "row.h"

#include "key.h"

Row::Row(XkbRowPtr row_, XkbDescPtr xkb_, QObject *parent)
    : XkbObject(xkb_, parent)
    , row(row_)
    , orientation(row->vertical ? Qt::Vertical : Qt::Horizontal)
    , bounds(QRect(row->bounds.x1, row->bounds.y1, row->bounds.x2, row->bounds.y2))
{
    for (int i = 0; i < row->num_keys; ++i) {
        keys.push_back(new Key(row->keys + i, xkb, this));
    }
}
