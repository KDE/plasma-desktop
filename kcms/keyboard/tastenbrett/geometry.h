/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include "xkbobject.h"

class Geometry : public XkbObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QObject *> doodads MEMBER doodads CONSTANT)
    Q_PROPERTY(QList<QObject *> sections MEMBER sections CONSTANT)
    Q_PROPERTY(qreal widthMM MEMBER widthMM CONSTANT)
    Q_PROPERTY(qreal heightMM MEMBER heightMM CONSTANT)
    Q_PROPERTY(QColor baseColor MEMBER baseColor CONSTANT)
    Q_PROPERTY(QColor labelColor MEMBER labelColor CONSTANT)

public:
    Geometry(XkbGeometryPtr geom_, XkbDescPtr xkb_, QObject *parent = nullptr);
    ~Geometry() override;

    XkbGeometryPtr geom = nullptr;

    QList<QObject *> doodads;
    QList<QObject *> sections;

    qreal widthMM = -1;
    qreal heightMM = -1;

    QColor baseColor;
    QColor labelColor;
};
