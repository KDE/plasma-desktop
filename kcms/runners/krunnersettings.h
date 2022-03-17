/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRUNNERSETTINGS_H
#define KRUNNERSETTINGS_H

#include <KActivities/Consumer>
#include <KConfigGroup>
#include <KQuickAddons/ManagedConfigModule>

class KRunnerSettingsBase;
class KRunnerSettingsData;

class KRunnerSettings : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(KRunnerSettingsBase *krunnerSettings READ krunnerSettings CONSTANT)
    /**
     * @return @c true if the KCM is not launched from the plugin settings, @c false
     * otherwise.
     */
    Q_PROPERTY(bool doesShowPluginButton MEMBER m_doesShowPluginButton CONSTANT)
    Q_PROPERTY(QString activityCount READ activityCount NOTIFY activityCountChanged)
    /**
     * @return @c true if activity aware is not enabled or only one activity has
     * history, @c false otherwise.
     */
    Q_PROPERTY(bool hasSingleHistory READ hasSingleHistory NOTIFY hasSingleHistoryChanged)
    Q_PROPERTY(QString singleActivityName READ singleActivityName NOTIFY singleActivityNameChanged)
    Q_PROPERTY(QStringList historyKeys READ historyKeys NOTIFY historyKeysChanged)

public:
    KRunnerSettings(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~KRunnerSettings() override;

    KRunnerSettingsBase *krunnerSettings() const;

    int activityCount() const;
    bool hasSingleHistory() const;
    QString singleActivityName() const;
    QStringList historyKeys() const;

    Q_INVOKABLE QString iconNameForActivity(const QString &id) const;

    Q_INVOKABLE void deleteAllHistory();
    Q_INVOKABLE void deleteHistoryGroup(const QString &key);

Q_SIGNALS:
    void activityCountChanged();
    void hasSingleHistoryChanged();
    void singleActivityNameChanged();
    void historyKeysChanged();

public Q_SLOTS:
    void save() override;

private:
    void configureClearHistoryButton();

    KRunnerSettingsData *m_data;
    KActivities::Consumer *m_consumer;
    KConfigGroup m_historyConfigGroup;
    QStringList m_historyKeys;

    bool m_doesShowPluginButton;

    const QString nullUuid = QStringLiteral("00000000-0000-0000-0000-000000000000");
};

#endif // KRUNNERSETTINGS_H
