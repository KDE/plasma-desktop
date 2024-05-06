/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include <KLocalizedString>
#include <KPluginModel>
#include <KQuickManagedConfigModule>
#include <KSharedConfig>

class KPluginWidget;
class KCMultiDialog;

class SearchConfigModule : public KQuickManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *model READ model CONSTANT)
    Q_PROPERTY(QString favoriteCategory MEMBER m_favoriteCategory CONSTANT)

public:
    explicit SearchConfigModule(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    QAbstractItemModel *model() const
    {
        return m_model;
    }
    static QStringList defaultFavoriteIds()
    {
        return {QStringLiteral("krunner_services"), QStringLiteral("krunner_systemsettings")};
    }

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;
    void reloadPlugins();
    void showKCM(const KPluginMetaData &data, const QVariantList args = {}, const KPluginMetaData &krunnerPluginData = {}) const;
    void showKRunnerKCM() const
    {
        showKCM(KPluginMetaData(QStringLiteral("plasma/kcms/desktop/kcm_krunnersettings")), {QStringLiteral("openedFromPluginSettings")});
    };
    void addToFavorites(const KPluginMetaData &data);
    void removeFromFavorites(const KPluginMetaData &data);
    void movePlugin(const KPluginMetaData &data, qsizetype destIndex);

private:
    void setDefaultIndicatorVisible(QWidget *widget, bool visible);
    QStringList getFavPluginIds() const
    {
        QStringList favIds;
        for (const KPluginMetaData &data : std::as_const(m_favoriteMetaDataList)) {
            favIds << data.pluginId();
        }
        return favIds;
    }
    void checkNeedsSave()
    {
        setNeedsSave(m_model->isSaveNeeded() || m_initialFavs != getFavPluginIds());
    }
    KPluginModel *m_model;
    KSharedConfigPtr m_config;
    QString m_pluginID;
    KCMultiDialog *m_krunnerSettingsDialog = nullptr;
    const QString m_favoriteCategory = i18n("Favorite Plugins");
    const QString m_normalCategory = i18n("Available Plugins");
    QList<KPluginMetaData> m_favoriteMetaDataList;
    QStringList m_initialFavs;
};
