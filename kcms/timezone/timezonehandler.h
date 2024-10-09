/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

class TimezoneHandler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList availableRegions READ availableRegions NOTIFY availableRegionsChanged)

    Q_PROPERTY(QUrl timezoneFileUrl READ timezoneFileUrl NOTIFY timezoneFileUrlChanged)

public:
    TimezoneHandler(QObject *parent = nullptr);
    QStringList availableRegions();
    QUrl timezoneFileUrl();

Q_SIGNALS:
    void availableRegionsChanged();
    void timezoneFileUrlChanged();

private:
};
