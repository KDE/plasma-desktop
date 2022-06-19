/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kcm.h"

#include <KAboutData>
#include <KCMultiDialog>
#include <KLocalizedString>
#include <KNSWidgets/Button>
#include <KPluginFactory>
#include <KPluginWidget>
#include <KRunner/RunnerManager>
#include <QDebug>

#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QLabel>
#include <QVBoxLayout>

#include "krunnerdata.h"

K_PLUGIN_FACTORY_WITH_JSON(SearchConfigModuleFactory, "kcm_plasmasearch.json", registerPlugin<SearchConfigModule>(); registerPlugin<KRunnerData>();)

SearchConfigModule::SearchConfigModule(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_config(KSharedConfig::openConfig("krunnerrc"))
{
    KAboutData *about = new KAboutData(QStringLiteral("kcm_search"),
                                       i18nc("kcm name for About dialog", "Configure search settings"),
                                       QStringLiteral("0.1"),
                                       QString(),
                                       KAboutLicense::LGPL);
    about->addAuthor(i18n("Vishesh Handa"), QString(), QStringLiteral("vhanda@kde.org"));
    setAboutData(about);
    setButtons(Apply | Default);

    if (!args.at(0).toString().isEmpty()) {
        m_pluginID = args.at(0).toString();
    }

    QVBoxLayout *layout = new QVBoxLayout(this);

    QHBoxLayout *headerLayout = new QHBoxLayout;
    layout->addLayout(headerLayout);

    QLabel *label = new QLabel(i18n("Enable or disable plugins (used in KRunner, Application Launcher, and the Overview effect)"));

    headerLayout->addWidget(label);
    headerLayout->addStretch();

    m_pluginSelector = new KPluginWidget(this);
    connect(m_pluginSelector, &KPluginWidget::changed, this, &SearchConfigModule::updateUnmanagedState);

    qDBusRegisterMetaType<QByteArrayList>();
    qDBusRegisterMetaType<QHash<QString, QByteArrayList>>();
    // This will trigger the reloadConfiguration method for the runner
    connect(m_pluginSelector, &KPluginWidget::pluginConfigSaved, this, [](const QString &componentName) {
        QDBusMessage message =
            QDBusMessage::createSignal(QStringLiteral("/krunnerrc"), QStringLiteral("org.kde.kconfig.notify"), QStringLiteral("ConfigChanged"));
        const QHash<QString, QByteArrayList> changes = {
            {QStringLiteral("Runners"), {componentName.toLocal8Bit()}},
        };
        message.setArguments({QVariant::fromValue(changes)});
        QDBusConnection::sessionBus().send(message);
    });

    layout->addWidget(m_pluginSelector);

    QHBoxLayout *downloadLayout = new QHBoxLayout;

    // Open KRunner settings
    m_krunnerSettingsButton = new QPushButton(QIcon::fromTheme(QStringLiteral("krunner")), i18n("Configure KRunner…"), this);
    connect(m_krunnerSettingsButton, &QPushButton::clicked, this, [this] {
        if (!m_krunnerSettingsDialog) {
            m_krunnerSettingsDialog = new KCMultiDialog(this);
            m_krunnerSettingsDialog->addModule(KPluginMetaData(QStringLiteral("plasma/kcms/desktop/kcm_krunnersettings")),
                                               {QStringLiteral("openedFromPluginSettings")});
        }

        m_krunnerSettingsDialog->show();
    });

    KNSWidgets::Button *downloadButton = new KNSWidgets::Button(i18n("Get New Plugins…"), QStringLiteral("krunner.knsrc"), this);
    connect(downloadButton, &KNSWidgets::Button::dialogFinished, this, [this](const QList<KNSCore::Entry> &changedEntries) {
        if (!changedEntries.isEmpty()) {
            m_pluginSelector->clear();
            m_pluginSelector->addPlugins(Plasma::RunnerManager::runnerMetaDataList(), i18n("Available Plugins"));
        }
    });
    downloadLayout->addStretch();
    downloadLayout->addWidget(m_krunnerSettingsButton);
    downloadLayout->addWidget(downloadButton);
    layout->addLayout(downloadLayout);

    connect(this, &SearchConfigModule::defaultsIndicatorsVisibleChanged, this, &SearchConfigModule::updateUnmanagedState);
    connect(this, &SearchConfigModule::defaultsIndicatorsVisibleChanged, m_pluginSelector, &KPluginWidget::setDefaultsIndicatorsVisible);
}

void SearchConfigModule::load()
{
    m_pluginSelector->clear();
    KCModule::load();

    // Set focus on the pluginselector to pass focus to search bar.
    m_pluginSelector->setFocus(Qt::OtherFocusReason);

    m_pluginSelector->addPlugins(Plasma::RunnerManager::runnerMetaDataList(), i18n("Available Plugins"));
    m_pluginSelector->setConfig(m_config->group("Plugins"));

    if (!m_pluginID.isEmpty()) {
        m_pluginSelector->showConfiguration(m_pluginID);
    }
}

void SearchConfigModule::save()
{
    KCModule::save();

    m_pluginSelector->save();

    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/krunnerrc"), QStringLiteral("org.kde.kconfig.notify"), QStringLiteral("ConfigChanged"));
    const QHash<QString, QByteArrayList> changes = {{QStringLiteral("Plugins"), {}}};
    message.setArguments({QVariant::fromValue(changes)});
    QDBusConnection::sessionBus().send(message);
}

void SearchConfigModule::defaults()
{
    KCModule::defaults();

    m_pluginSelector->defaults();
}

void SearchConfigModule::updateUnmanagedState()
{
    unmanagedWidgetChangeState(m_pluginSelector->isSaveNeeded());
    unmanagedWidgetDefaultState(m_pluginSelector->isDefault());
}

void SearchConfigModule::setDefaultIndicatorVisible(QWidget *widget, bool visible)
{
    widget->setProperty("_kde_highlight_neutral", visible);
    widget->update();
}

#include "kcm.moc"
