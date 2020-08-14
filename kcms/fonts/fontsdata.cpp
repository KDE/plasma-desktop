/*
 * Copyright (c) 2020 Benjamin Port <benjamin.port@enioka.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "fontsdata.h"

#include <QVariantList>

#include "fontssettings.h"
#include "fontsaasettings.h"

FontsData::FontsData(QObject *parent, const QVariantList &args)
    : KCModuleData(parent, args)
    , m_settings(new FontsSettings(this))
    , m_settingsAA(new FontsAASettings(this))
{
    autoRegisterSkeletons();
}

FontsSettings *FontsData::settings() const
{
    return m_settings;
}

FontsAASettings *FontsData::settingsAA() const
{
    return m_settingsAA;
}

#include "fontsdata.moc"
