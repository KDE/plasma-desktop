/*
    "Desktop Options" Tab for KDesktop configuration

    SPDX-FileCopyrightText: 1996 Martin R. Jones
    SPDX-FileCopyrightText: 1998 Bernd Wuebben
    SPDX-FileCopyrightText: 1998 Christian Tibirna
    SPDX-FileCopyrightText: 1998-2007 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2010 Matthias Fuchs <mat69@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "globalpaths.h"
#include "desktoppathsdata.h"
#include "desktoppathssettings.h"

#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(DesktopPathConfig, "kcm_desktoppaths.json")

DesktopPathConfig::DesktopPathConfig(QObject *parent, const KPluginMetaData &metaData)
    : KQuickManagedConfigModule(parent, metaData)
    , m_data(new DesktopPathsData(this))
{
}

DesktopPathConfig::~DesktopPathConfig()
{
}

QObject *DesktopPathConfig::settings() const
{
    return m_data->settings();
}

bool DesktopPathConfig::isDefaults() const
{
    return m_data->isDefaults();
}

#include "globalpaths.moc"

#include "moc_globalpaths.cpp"
