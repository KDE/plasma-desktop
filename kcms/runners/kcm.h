/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef _KCM_SEARCH_H
#define _KCM_SEARCH_H

#include <KCModule>
#include <KSharedConfig>
#include <QPushButton>

class KPluginWidget;
class KCMultiDialog;

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

private:
    void setDefaultIndicatorVisible(QWidget *widget, bool visible);

    KPluginWidget *m_pluginSelector;
    KSharedConfigPtr m_config;
    QString m_pluginID;
    QPushButton *m_krunnerSettingsButton = nullptr;
    KCMultiDialog *m_krunnerSettingsDialog = nullptr;
};

#endif
