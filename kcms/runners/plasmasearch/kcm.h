/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef _KCM_SEARCH_H
#define _KCM_SEARCH_H

#include <KPluginModel>
#include <KQuickAddons/ManagedConfigModule>
#include <KSharedConfig>

class KPluginWidget;
class KCMultiDialog;

class SearchConfigModule : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *model READ model CONSTANT)

public:
    explicit SearchConfigModule(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    QAbstractItemModel *model() const
    {
        return m_model;
    }

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;
    void reloadPlugins();
    void showKCM(const KPluginMetaData &data, const QStringList args = {}, const KPluginMetaData &krunnerPluginData = {}) const;
    void showKRunnerKCM() const
    {
        showKCM(KPluginMetaData(QStringLiteral("plasma/kcms/desktop/kcm_krunnersettings")), {QStringLiteral("openedFromPluginSettings")});
    };

private:
    void setDefaultIndicatorVisible(QWidget *widget, bool visible);

    KPluginModel *m_model;
    KSharedConfigPtr m_config;
    QString m_pluginID;
    KCMultiDialog *m_krunnerSettingsDialog = nullptr;
};

#endif
