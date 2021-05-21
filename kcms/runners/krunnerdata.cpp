/*
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
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
