/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QIcon>
#include <QMap>
#include <QObject>
#include <QString>

class LayoutUnit;
class KeyboardConfig;

class Flags : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

    Q_INVOKABLE QIcon getIcon(const QString &layout);

    static QString getLongText(const LayoutUnit &layoutUnit);
    static QString getShortText(const LayoutUnit &layoutUnit, const KeyboardConfig &keyboardConfig);

private:
    QIcon createIcon(const QString &layout);
    QString getCountryFromLayoutName(const QString &fullLayoutName) const;

    QMap<QString, QIcon> iconMap;
};
