/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef _KCM_SEARCH_H
#define _KCM_SEARCH_H

#include <KActivities/Consumer>
#include <KCModule>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QCheckBox>
#include <QPushButton>
#include <QRadioButton>
#include <QToolButton>

class KPluginSelector;
class KRunnerSettings;

class SearchConfigModule : public KCModule
{
    Q_OBJECT

public:
    enum Roles {
        RunnersRole = Qt::UserRole + 1,
        DescriptionRole,
    };

    SearchConfigModule(QWidget *parent, const QVariantList &args);

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;
    void updateUnmanagedState();
    void configureClearHistoryButton();
    void deleteHistoryGroup(const QString &key);
    void deleteAllHistory();

private:
    void setDefaultIndicatorVisible(QWidget *widget, bool visible);

    KPluginSelector *m_pluginSelector;
    KSharedConfigPtr m_config;
    QString m_pluginID;
    QRadioButton *m_topPositioning;
    QRadioButton *m_freeFloating;
    QCheckBox *m_retainPriorSearch;
    QCheckBox *m_activityAware;
    QToolButton *m_clearHistoryButton;
    QCheckBox *m_enableHistory;
    KRunnerSettings *m_settings;
    KActivities::Consumer *m_consumer;
    KConfigGroup m_historyConfigGroup;
    const QString nullUuid = QStringLiteral("00000000-0000-0000-0000-000000000000");
};

#endif
