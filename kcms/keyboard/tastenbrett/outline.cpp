/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "outline.h"

#include <QPoint>
#include <QVariant>

Outline::Outline(XkbOutlinePtr outline_, XkbDescPtr xkb_, QObject *parent)
    : XkbObject(xkb_, parent)
    , outline(outline_)
{
    for (int i = 0; i < outline->num_points; ++i) {
        const auto p = outline->points + i;
        points.push_back(QPoint(p->x, p->y));
    }
}
