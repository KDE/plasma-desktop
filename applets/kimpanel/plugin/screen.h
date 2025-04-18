/*
    SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QObject>
#include <QRect>
#include <qqmlregistration.h>

class Screen : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    Screen(QObject *parent = nullptr);
    ~Screen() override;

    Q_INVOKABLE QRect geometryForPoint(int x, int y);
    Q_INVOKABLE qreal devicePixelRatioForPoint(int x, int y);
};
