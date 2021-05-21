/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SHAPE_H
#define SHAPE_H

#include <QRect>

#include "xkbobject.h"

class Shape : public XkbObject
{
    Q_OBJECT

    Q_PROPERTY(QList<QObject *> outlines MEMBER outlines CONSTANT)
    Q_PROPERTY(QRect bounds MEMBER bounds CONSTANT)
public:
    Shape(XkbShapePtr shape_, XkbDescPtr xkb_, QObject *parent = nullptr);

    XkbShapePtr shape = nullptr;
    QList<QObject *> outlines;
    QRect bounds;
};

#endif // SHAPE_H
