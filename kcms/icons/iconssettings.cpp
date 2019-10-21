/*
 * Copyright (c) 2019 Benjamin Port <benjamin.port@enioka.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QObject>
#include <QDebug>

#include <KBuildSycocaProgressDialog>
#include <KIconTheme>
#include <KSharedDataCache>

#include "iconssettings.h"

IconsSettings::IconsSettings(QObject *parent)
        : IconsSettingsBase(parent)
        , m_themeDirty(false)
{
    connect(this, &IconsSettings::configChanged, this, &IconsSettings::updateIconTheme);
    connect(this, &IconsSettings::ThemeChanged, this, &IconsSettings::updateThemeDirty);
}

IconsSettings::~IconsSettings()
{
}

void IconsSettings::updateThemeDirty()
{
    m_themeDirty = theme() != KIconTheme::current();
}

void IconsSettings::updateIconTheme()
{
    if (m_themeDirty) {
        KIconTheme::reconfigure();

        KSharedDataCache::deleteCache(QStringLiteral("icon-cache"));

        for (int i = 0; i < KIconLoader::LastGroup; i++) {
            KIconLoader::emitChange(KIconLoader::Group(i));
        }

        KBuildSycocaProgressDialog::rebuildKSycoca(nullptr);
    }
}
