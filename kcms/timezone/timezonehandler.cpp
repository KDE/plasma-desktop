/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QFile>
#include <QLocale>
#include <QStandardPaths>
#include <QTimeZone>
#include <QUrl>

#include "timezonehandler.h"

TimezoneHandler::TimezoneHandler(QObject *parent)
    : QObject(parent)
{
}

#include "moc_timezonehandler.cpp"

QStringList TimezoneHandler::availableRegions()
{
    QStringList stringList;
    for (const QByteArray &byteArray : QTimeZone::availableTimeZoneIds()) {
        stringList.append(QString::fromUtf8(byteArray));
    }
    return stringList;
}

QUrl TimezoneHandler::timezoneFileUrl()
{
    qDebug() << "!!!!" << QLocale::system().name().toLatin1();
    return QUrl::fromLocalFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "timezonefiles/timezones.json"));
}
