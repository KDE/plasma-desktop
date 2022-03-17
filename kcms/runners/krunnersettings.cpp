/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krunnersettings.h"

#include <KActivities/Info>
#include <KConfig>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QDBusConnection>
#include <QDBusMessage>

#include "krunnersettingsbase.h"
#include "krunnersettingsdata.h"

K_PLUGIN_CLASS_WITH_JSON(KRunnerSettings, "kcm_krunnersettings.json")

KRunnerSettings::KRunnerSettings(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, metaData, args)
    , m_data(new KRunnerSettingsData(this))
    , m_consumer(new KActivities::Consumer(this))
    , m_historyConfigGroup(KSharedConfig::openConfig(QStringLiteral("krunnerstaterc"), KConfig::NoGlobals, QStandardPaths::GenericDataLocation)
                               ->group("PlasmaRunnerManager")
                               .group("History"))
    , m_historyKeys(m_historyConfigGroup.keyList())
    , m_doesShowPluginButton(args.size() == 0 || args.constFirst() != QLatin1String("openedFromPluginSettings"))
{
    qmlRegisterType<KRunnerSettingsBase>();

    setButtons(Apply | Default);

    connect(krunnerSettings(), &KRunnerSettingsBase::activityAwareChanged, this, &KRunnerSettings::hasSingleHistoryChanged);
    connect(m_consumer, &KActivities::Consumer::activitiesChanged, this, &KRunnerSettings::activityCountChanged);
    connect(m_consumer, &KActivities::Consumer::activitiesChanged, this, &KRunnerSettings::hasSingleHistoryChanged);
}

KRunnerSettings::~KRunnerSettings()
{
}

KRunnerSettingsBase *KRunnerSettings::krunnerSettings() const
{
    return m_data->settings();
}

int KRunnerSettings::activityCount() const
{
    return m_consumer->activities().size();
}

bool KRunnerSettings::hasSingleHistory() const
{
    return !krunnerSettings()->activityAware() || m_historyKeys.size() == 1 || activityCount() == 1;
}

QStringList KRunnerSettings::historyKeys() const
{
    return m_historyKeys;
}

QString KRunnerSettings::iconNameForActivity(const QString &id) const
{
    return KActivities::Info(id).icon();
}

QString KRunnerSettings::singleActivityName() const
{
    if (m_historyKeys.size() == 0) {
        return QString();
    }

    // Sometimes there are history items in deleted activities but only
    // one activity is available.
    if (const QStringList activities = m_consumer->activities(); activities.size() == 1) {
        return KActivities::Info(activities.constFirst()).name();
    }

    return KActivities::Info(m_historyKeys.constFirst()).name();
}

void KRunnerSettings::configureClearHistoryButton()
{
    const QStringList historyKeys = m_historyConfigGroup.keyList();

    if (historyKeys == m_historyKeys) {
        return;
    }

    m_historyKeys = historyKeys;

    Q_EMIT hasSingleHistoryChanged();
    Q_EMIT singleActivityNameChanged();
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
    ManagedConfigModule::save();

    // Combine & write history
    if (!krunnerSettings()->activityAware()) {
        if (!m_historyConfigGroup.hasKey(nullUuid)) {
            QStringList activities = m_consumer->activities();
            activities.removeOne(m_consumer->currentActivity());
            QStringList newHistory = m_historyConfigGroup.readEntry(m_consumer->currentActivity(), QStringList());
            for (const QString &activity : std::as_const(activities)) {
                newHistory.append(m_historyConfigGroup.readEntry(activity, QStringList()));
            }
            newHistory.removeDuplicates();
            m_historyConfigGroup.writeEntry(nullUuid, newHistory, KConfig::Notify);
            m_historyConfigGroup.sync();
        }
    }

    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/krunnerrc"), QStringLiteral("org.kde.kconfig.notify"), QStringLiteral("ConfigChanged"));
    const QHash<QString, QByteArrayList> changes = {{QStringLiteral("Plugins"), {}}};
    message.setArguments({QVariant::fromValue(changes)});
    QDBusConnection::sessionBus().send(message);
}

#include "krunnersettings.moc"
