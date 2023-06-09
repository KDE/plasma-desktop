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
#include <KLocalizedString>
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
        const QVector<KPluginMetaData> runnerData = KRunner::RunnerManager::runnerMetaDataList();
        KConfigGroup cfgGroup(m_krunnerConfig, "Plugins");
        return std::all_of(runnerData.begin(), runnerData.end(), [&cfgGroup](const KPluginMetaData &pluginData) {
            return pluginData.isEnabled(cfgGroup) != pluginData.isEnabledByDefault();
        });
    }

private:
    KSharedConfigPtr m_krunnerConfig = KSharedConfig::openConfig("krunnerrc");
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
        setNeedsSave(m_model->isSaveNeeded());
        setRepresentsDefaults(isDefaults);
    });
    connect(m_model, &KPluginModel::isSaveNeededChanged, this, [this]() {
        setNeedsSave(m_model->isSaveNeeded());
    });
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
    m_model->addPlugins(KRunner::RunnerManager::runnerMetaDataList(), i18n("Available Plugins"));
}

void SearchConfigModule::showKCM(const KPluginMetaData &data, const QVariantList args, const KPluginMetaData &krunnerPluginData) const
{
    auto dlg = new KCMultiDialog();
    dlg->addModule(data, args);
    dlg->show();
    connect(dlg, qOverload<>(&KCMultiDialog::configCommitted), dlg, [krunnerPluginData]() {
        QDBusMessage message =
            QDBusMessage::createSignal(QStringLiteral("/krunnerrc"), QStringLiteral("org.kde.kconfig.notify"), QStringLiteral("ConfigChanged"));
        const QHash<QString, QByteArrayList> changes = {
            {QStringLiteral("Runners"), {krunnerPluginData.pluginId().toLocal8Bit()}},
        };
        message.setArguments({QVariant::fromValue(changes)});
        QDBusConnection::sessionBus().send(message);
    });
}

void SearchConfigModule::save()
{
    KQuickManagedConfigModule::save();

    m_model->save();

    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/krunnerrc"), QStringLiteral("org.kde.kconfig.notify"), QStringLiteral("ConfigChanged"));
    const QHash<QString, QByteArrayList> changes = {{QStringLiteral("Plugins"), {}}};
    message.setArguments({QVariant::fromValue(changes)});
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
