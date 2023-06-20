/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KConfigGroup>
#include <KQuickManagedConfigModule>

class KRunnerSettingsBase;
class KRunnerSettingsData;

class KRunnerSettings : public KQuickManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(KRunnerSettingsBase *krunnerSettings READ krunnerSettings CONSTANT)
    /**
     * @return @c true if the KCM is not launched from the plugin settings, @c false
     * otherwise.
     */
    Q_PROPERTY(bool doesShowPluginButton MEMBER m_doesShowPluginButton CONSTANT)
    Q_PROPERTY(QStringList historyKeys READ historyKeys NOTIFY historyKeysChanged)

public:
    KRunnerSettings(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);

    KRunnerSettingsBase *krunnerSettings() const;

    QStringList historyKeys() const;

    Q_INVOKABLE void deleteAllHistory();
    Q_INVOKABLE void deleteHistoryGroup(const QString &key);

Q_SIGNALS:
    void historyKeysChanged();

public Q_SLOTS:
    void save() override;

private:
    void configureClearHistoryButton();

    KRunnerSettingsData *m_data;
    KConfigGroup m_historyConfigGroup;
    QStringList m_historyKeys;

    bool m_doesShowPluginButton;

    const QString nullUuid = QStringLiteral("00000000-0000-0000-0000-000000000000");
};
