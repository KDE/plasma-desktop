/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020, 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kcm.h"

#include <KCModuleData>
#include <KCMultiDialog>
#include <KConfigGroup>
#include <KPluginFactory>
#include <KRunner/RunnerManager>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>

class KRunnerData : public KCModuleData
{
    Q_OBJECT
public:
    using KCModuleData::KCModuleData;

    bool isDefaults() const override
    {
        const QList<KPluginMetaData> runnerData = KRunner::RunnerManager::runnerMetaDataList();
        KConfigGroup cfgGroup(m_krunnerConfig, QStringLiteral("Plugins"));
        if (cfgGroup.group("Favorites").readEntry("plugins", SearchConfigModule::defaultFavoriteIds()) != SearchConfigModule::defaultFavoriteIds()) {
            return false;
        }
        return std::all_of(runnerData.begin(), runnerData.end(), [&cfgGroup](const KPluginMetaData &pluginData) {
            return pluginData.isEnabled(cfgGroup) != pluginData.isEnabledByDefault();
        });
    }

private:
    const KSharedConfigPtr m_krunnerConfig = KSharedConfig::openConfig("krunnerrc");
};

K_PLUGIN_FACTORY_WITH_JSON(SearchConfigModuleFactory, "kcm_plasmasearch.json", registerPlugin<SearchConfigModule>(); registerPlugin<KRunnerData>();)

SearchConfigModule::SearchConfigModule(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KQuickManagedConfigModule(parent, data)
    , m_model(new KPluginModel(this))
    , m_config(KSharedConfig::openConfig("krunnerrc"))
{
    qDBusRegisterMetaType<QByteArrayList>();
    qDBusRegisterMetaType<QHash<QString, QByteArrayList>>();

    if (!args.isEmpty()) {
        m_pluginID = args.at(0).toString();
    }
    qDBusRegisterMetaType<QByteArrayList>();
    qDBusRegisterMetaType<QHash<QString, QByteArrayList>>();

    connect(m_model, &KPluginModel::defaulted, this, [this](bool isDefaults) {
        checkNeedsSave();
        setRepresentsDefaults(isDefaults);
    });
    connect(m_model, &KPluginModel::isSaveNeededChanged, this, &SearchConfigModule::checkNeedsSave);
}

void SearchConfigModule::load()
{
    reloadPlugins();

    if (!m_pluginID.isEmpty()) {
        const KPluginMetaData data = m_model->findConfigForPluginId(m_pluginID);
        if (data.isValid()) {
            showKCM(data);
        } else {
            qWarning() << "Could not find plugin with id" << m_pluginID;
        }
        m_pluginID.clear(); // Clear this to avoid showing the plugin's config again if we reset the KCM
    }
}

void SearchConfigModule::reloadPlugins()
{
    m_model->clear();

    m_model->setConfig(m_config->group("Plugins"));
    m_initialFavs = m_config->group("Plugins").group("Favorites").readEntry("plugins", defaultFavoriteIds());

    QList<KPluginMetaData> metaDataList = KRunner::RunnerManager::runnerMetaDataList();
    auto it = std::partition(metaDataList.begin(), metaDataList.end(), [this](const KPluginMetaData &plugin) {
        return m_initialFavs.contains(plugin.pluginId());
    });

    m_favoriteMetaDataList = QList<KPluginMetaData>(metaDataList.begin(), it);
    std::sort(m_favoriteMetaDataList.begin(), m_favoriteMetaDataList.end(), [this](const KPluginMetaData &data1, const KPluginMetaData &data2) {
        return m_initialFavs.indexOf(data1.pluginId()) < m_initialFavs.indexOf(data2.pluginId());
    });
    m_model->addUnsortablePlugins(m_favoriteMetaDataList, m_favoriteCategory);
    m_model->addPlugins(QList<KPluginMetaData>(it, metaDataList.end()), m_normalCategory);
    setNeedsSave(false);
}
void SearchConfigModule::addToFavorites(const KPluginMetaData &data)
{
    m_model->removePlugin(data);
    m_favoriteMetaDataList << data;
    m_model->addUnsortablePlugins({data}, m_favoriteCategory);
    checkNeedsSave();
}
void SearchConfigModule::removeFromFavorites(const KPluginMetaData &data)
{
    m_model->removePlugin(data);
    m_favoriteMetaDataList.removeOne(data);
    m_model->addPlugins({data}, m_normalCategory);
    checkNeedsSave();
}

void SearchConfigModule::movePlugin(const KPluginMetaData &data, qsizetype destIndex)
{
    if (destIndex >= m_favoriteMetaDataList.size()) {
        // Dragged outside favorties section, snap it back to the end.
        destIndex = m_favoriteMetaDataList.size() - 1;
    }

    m_favoriteMetaDataList.removeOne(data);
    m_favoriteMetaDataList.insert(destIndex, data);
    checkNeedsSave();

    for (int row = 0; row < m_model->rowCount(); ++row) {
        QModelIndex index = m_model->index(row, 0); // 0 for column as we don't have multiple columns
        QVariant value = m_model->data(index, KPluginModel::IdRole);
        if (value == data.pluginId()) {
            m_model->moveRow(QModelIndex(), index.row(), QModelIndex(), destIndex);
            return;
        }
    }
}

void SearchConfigModule::showKCM(const KPluginMetaData &data, const QVariantList args, const KPluginMetaData &krunnerPluginData) const
{
    auto dlg = new KCMultiDialog();
    dlg->addModule(data, args);
    dlg->show();
    connect(dlg, qOverload<>(&KCMultiDialog::configCommitted), dlg, [krunnerPluginData]() {
        QDBusMessage message =
            QDBusMessage::createSignal(QStringLiteral("/krunnerrc"), QStringLiteral("org.kde.kconfig.notify"), QStringLiteral("ConfigChanged"));
        QHash<QString, QByteArrayList> changes = {
            {QStringLiteral("Runners"), {krunnerPluginData.pluginId().toLocal8Bit()}},
        };
        message.setArguments({QVariant::fromValue(std::move(changes))});
        QDBusConnection::sessionBus().send(message);
    });
}

void SearchConfigModule::save()
{
    KQuickManagedConfigModule::save();
    KConfigGroup grp = m_config->group(QStringLiteral("Plugins")).group(QStringLiteral("Favorites"));
    grp.writeEntry("plugins", getFavPluginIds(), KConfigGroup::Notify);
    m_model->save();

    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/krunnerrc"), QStringLiteral("org.kde.kconfig.notify"), QStringLiteral("ConfigChanged"));
    QHash<QString, QByteArrayList> changes{{QStringLiteral("Plugins"), {}}};
    message.setArguments({QVariant::fromValue(std::move(changes))});
    QDBusConnection::sessionBus().send(message);
}

void SearchConfigModule::defaults()
{
    KQuickManagedConfigModule::defaults();

    m_model->defaults();
}

void SearchConfigModule::setDefaultIndicatorVisible(QWidget *widget, bool visible)
{
    widget->setProperty("_kde_highlight_neutral", visible);
    widget->update();
}

#include "kcm.moc"

#include "moc_kcm.cpp"
