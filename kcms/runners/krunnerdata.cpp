/*
   Copyright (c) 2020 Cyril Rossi <cyril.rossi@enioka.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "krunnerdata.h"

#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <KPackage/PackageStructure>
#include <KPluginFactory>
#include <KPluginInfo>
#include <KRunner/RunnerManager>

#include "krunnersettings.h"

KRunnerData::KRunnerData(QObject *parent, const QVariantList &args)
    : KCModuleData(parent, args)
    , m_krunnerConfig(KSharedConfig::openConfig("krunnerrc"))
    , m_settings(new KRunnerSettings(this))
{
    m_settings->load();
}

bool KRunnerData::isDefaults() const
{
    QList<KPluginInfo> runnerInfos = KPluginInfo::fromMetaData(Plasma::RunnerManager::runnerMetaDataList());
    KConfigGroup cfgGroup(m_krunnerConfig, "Plugins");
    for (auto &plugin : runnerInfos) {
        plugin.load(cfgGroup);
        if (plugin.isPluginEnabled() != plugin.isPluginEnabledByDefault()) {
            return false;
        }
    }

    return m_settings->isDefaults();
}
