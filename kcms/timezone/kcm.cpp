/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm.h"
#include "timezonehandler.h"

#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(KCMTimeZone, "kcm_timezone.json")

KCMTimeZone::KCMTimeZone(QObject *parent, const KPluginMetaData &metaData)
    : KQuickManagedConfigModule(parent, metaData)
{
    setButtons(Help);

    TimezoneHandler th = TimezoneHandler();

    constexpr const char *uri{"org.kde.plasma.timezone.kcm"};

    qmlRegisterType<TimezoneHandler>(uri, 2, 1, "TimeZones");
}

KCMTimeZone::~KCMTimeZone()
{
}

#include "kcm.moc"
