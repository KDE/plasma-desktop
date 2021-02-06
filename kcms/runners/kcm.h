/*
   Copyright (c) 2014 Vishesh Handa <me@vhanda.in>
   Copyright (c) 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
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
