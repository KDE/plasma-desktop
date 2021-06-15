/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FLAGS_H_
#define FLAGS_H_

#include <QMap>
#include <QObject>
#include <QString>
#include <QIcon>

class QIcon;
class LayoutUnit;
class KeyboardConfig;
struct Rules;

class Flags : public QObject
{
    Q_OBJECT

public:
    const QIcon getIcon(const QString &layout);

    static QString getLongText(const LayoutUnit &layoutUnit, const Rules *rules);
    static QString getShortText(const LayoutUnit &layoutUnit, const KeyboardConfig &keyboardConfig);

private:
    QIcon createIcon(const QString &layout);
    QString getCountryFromLayoutName(const QString &fullLayoutName) const;

    QMap<QString, QIcon> iconMap;
};

#endif /* FLAGS_H_ */
