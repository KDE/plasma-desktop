/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef ROW_H
#define ROW_H

#include <QRect>

#include "xkbobject.h"

class Row : public XkbObject
{
    Q_OBJECT

#define R_P(type, name)                                                                                                                                        \
private:                                                                                                                                                       \
    Q_PROPERTY(type name READ auto_prop_##name CONSTANT)                                                                                                       \
public:                                                                                                                                                        \
    type auto_prop_##name() const                                                                                                                              \
    {                                                                                                                                                          \
        return row->name;                                                                                                                                      \
    }

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
