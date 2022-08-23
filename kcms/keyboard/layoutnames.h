/*
    SPDX-FileCopyrightText: 2021 Andrey Butirsky <butirsky@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LAYOUTNAMES_H
#define LAYOUTNAMES_H

#include <QMetaType>

class QDBusArgument;

struct LayoutNames {
    static void registerMetaType();

    QString shortName;
    QString displayName;
    QString longName;
};
Q_DECLARE_METATYPE(LayoutNames)

QDBusArgument &operator<<(QDBusArgument &argument, const LayoutNames &layoutNames);
const QDBusArgument &operator>>(const QDBusArgument &argument, LayoutNames &layoutNames);

#endif // LAYOUTNAMES_H
