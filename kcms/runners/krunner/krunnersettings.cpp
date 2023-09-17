/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krunnersettings.h"

#include <KLocalizedString>
#include <KPluginFactory>

#include <QDBusConnection>
#include <QDBusMessage>

#include "krunnersettingsbase.h"
#include "krunnersettingsdata.h"

K_PLUGIN_CLASS_WITH_JSON(KRunnerSettings, "kcm_krunnersettings.json")

KRunnerSettings::KRunnerSettings(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KQuickManagedConfigModule(parent, metaData)
    , m_data(new KRunnerSettingsData(this))
    , m_historyConfigGroup(KSharedConfig::openConfig(QStringLiteral("krunnerstaterc"), KConfig::NoGlobals, QStandardPaths::GenericDataLocation)
                               ->group("PlasmaRunnerManager")
                               .group("History"))
    , m_historyKeys(m_historyConfigGroup.keyList())
    , m_doesShowPluginButton(args.isEmpty() || args.constFirst() != QLatin1String("openedFromPluginSettings"))
{
    qmlRegisterUncreatableType<KRunnerSettingsBase>("org.kde.plasma.runners.kcm", 1, 0, "KRunnerSettings", QStringLiteral("Only for enums"));

    setButtons(Apply | Default);
}

KRunnerSettingsBase *KRunnerSettings::krunnerSettings() const
{
    return m_data->settings();
}

QStringList KRunnerSettings::historyKeys() const
{
    return m_historyKeys;
}

void KRunnerSettings::configureClearHistoryButton()
{
    const QStringList historyKeys = m_historyConfigGroup.keyList();

    if (historyKeys == m_historyKeys) {
        return;
    }

    m_historyKeys = historyKeys;

    Q_EMIT historyKeysChanged();
}

void KRunnerSettings::deleteAllHistory()
{
    m_historyConfigGroup.deleteGroup(KConfig::Notify);
    m_historyConfigGroup.sync();
    configureClearHistoryButton();
}

void KRunnerSettings::deleteHistoryGroup(const QString &key)
{
    if (key.isEmpty()) {
        return;
    }

    m_historyConfigGroup.deleteEntry(key, KConfig::Notify);
    m_historyConfigGroup.sync();
    configureClearHistoryButton();
}

void KRunnerSettings::save()
{
    KQuickManagedConfigModule::save();

    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/krunnerrc"), QStringLiteral("org.kde.kconfig.notify"), QStringLiteral("ConfigChanged"));
    QHash<QString, QByteArrayList> changes = {{QStringLiteral("Plugins"), {}}};
    message.setArguments({QVariant::fromValue(std::move(changes))});
    QDBusConnection::sessionBus().send(message);
}

#include "krunnersettings.moc"
#include "moc_krunnersettings.cpp"
