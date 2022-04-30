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

KRunnerData::KRunnerData(QObject *parent, const QVariantList &args)
    : KCModuleData(parent, args)
    , m_krunnerConfig(KSharedConfig::openConfig("krunnerrc"))
{
}

bool KRunnerData::isDefaults() const
{
    const QVector<KPluginMetaData> runnerData = Plasma::RunnerManager::runnerMetaDataList();
    KConfigGroup cfgGroup(m_krunnerConfig, "Plugins");

    if (std::any_of(runnerData.cbegin(), runnerData.cend(), [&cfgGroup](const KPluginMetaData &pluginData) {
            return pluginData.isEnabled(cfgGroup) != pluginData.isEnabledByDefault();
        })) {
        return false;
    }

    return true;
}
