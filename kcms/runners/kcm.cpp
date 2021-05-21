/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kcm.h"

#include <KAboutData>
#include <KActivities/Consumer>
#include <KActivities/Info>
#include <KLocalizedString>
#include <KNS3/Button>
#include <KPluginFactory>
#include <KPluginSelector>
#include <KRunner/RunnerManager>
#include <QDebug>
#include <QStandardPaths>

#include "krunnerdata.h"
#include "krunnersettings.h"
#include <QAction>
#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QToolButton>
#include <QVBoxLayout>

K_PLUGIN_FACTORY(SearchConfigModuleFactory, registerPlugin<SearchConfigModule>(); registerPlugin<KRunnerData>();)

SearchConfigModule::SearchConfigModule(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_config(KSharedConfig::openConfig("krunnerrc"))
    , m_settings(new KRunnerSettings(this))
{
    KAboutData *about = new KAboutData(QStringLiteral("kcm_search"),
                                       i18nc("kcm name for About dialog", "Configure Search Bar"),
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
    m_consumer = new KActivities::Consumer(this);
    m_historyConfigGroup = KSharedConfig::openConfig(QStringLiteral("krunnerstaterc"), KConfig::NoGlobals, QStandardPaths::GenericDataLocation)
                               ->group("PlasmaRunnerManager")
                               .group("History");

    QHBoxLayout *headerLayout = new QHBoxLayout;
    layout->addLayout(headerLayout);

    QLabel *label = new QLabel(i18n("Enable or disable plugins (used in KRunner and Application Launcher)"));

    m_clearHistoryButton = new QToolButton(this);
    m_clearHistoryButton->setIcon(
        QIcon::fromTheme(isRightToLeft() ? QStringLiteral("edit-clear-locationbar-ltr") : QStringLiteral("edit-clear-locationbar-rtl")));
    m_clearHistoryButton->setPopupMode(QToolButton::InstantPopup);
    m_clearHistoryButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    connect(m_clearHistoryButton, &QPushButton::clicked, this, &SearchConfigModule::deleteAllHistory);

    QHBoxLayout *configHeaderLayout = new QHBoxLayout;
    QVBoxLayout *configHeaderLeft = new QVBoxLayout;
    QVBoxLayout *configHeaderRight = new QVBoxLayout;

    // Options where KRunner should pop up
    m_topPositioning = new QRadioButton(i18n("Top"), this);
    connect(m_topPositioning, &QRadioButton::toggled, this, &SearchConfigModule::updateUnmanagedState);
    m_freeFloating = new QRadioButton(i18n("Center"), this);
    connect(m_freeFloating, &QRadioButton::toggled, this, &SearchConfigModule::updateUnmanagedState);

    QFormLayout *positionLayout = new QFormLayout;
    positionLayout->addRow(i18n("Position on screen:"), m_topPositioning);
    positionLayout->addRow(QString(), m_freeFloating);
    m_enableHistory = new QCheckBox(i18n("Enable"), this);
    m_enableHistory->setObjectName("kcfg_historyEnabled");
    positionLayout->addItem(new QSpacerItem(0, 0));
    positionLayout->addRow(i18n("History:"), m_enableHistory);
    connect(m_enableHistory, &QCheckBox::toggled, m_clearHistoryButton, &QPushButton::setEnabled);
    m_retainPriorSearch = new QCheckBox(i18n("Retain previous search"), this);
    m_retainPriorSearch->setObjectName("kcfg_retainPriorSearch");
    positionLayout->addRow(QString(), m_retainPriorSearch);
    m_activityAware = new QCheckBox(i18n("Activity aware (previous search and history)"), this);
    m_activityAware->setObjectName("kcfg_activityAware");
    connect(m_activityAware, &QCheckBox::clicked, this, &SearchConfigModule::configureClearHistoryButton);
    positionLayout->addRow(QString(), m_activityAware);
    configHeaderLeft->addLayout(positionLayout);

    configHeaderRight->setSizeConstraint(QLayout::SetNoConstraint);
    configHeaderRight->setAlignment(Qt::AlignBottom);
    configHeaderRight->addWidget(m_clearHistoryButton);

    configHeaderLayout->addLayout(configHeaderLeft);
    configHeaderLayout->addStretch();
    configHeaderLayout->addLayout(configHeaderRight);

    headerLayout->addWidget(label);
    headerLayout->addStretch();

    m_pluginSelector = new KPluginSelector(this);
    connect(m_pluginSelector, &KPluginSelector::changed, this, &SearchConfigModule::updateUnmanagedState);

    qDBusRegisterMetaType<QByteArrayList>();
    qDBusRegisterMetaType<QHash<QString, QByteArrayList>>();
    // This will trigger the reloadConfiguration method for the runner
    connect(m_pluginSelector, &KPluginSelector::configCommitted, this, [](const QByteArray &componentName) {
        QDBusMessage message =
            QDBusMessage::createSignal(QStringLiteral("/krunnerrc"), QStringLiteral("org.kde.kconfig.notify"), QStringLiteral("ConfigChanged"));
        const QHash<QString, QByteArrayList> changes = {{QStringLiteral("Runners"), {componentName}}};
        message.setArguments({QVariant::fromValue(changes)});
        QDBusConnection::sessionBus().send(message);
    });

    layout->addLayout(configHeaderLayout);
    layout->addSpacing(12);
    layout->addWidget(m_pluginSelector);

    QHBoxLayout *downloadLayout = new QHBoxLayout;
    KNS3::Button *downloadButton = new KNS3::Button(i18n("Get New Plugins…"), QStringLiteral("krunner.knsrc"), this);
    connect(downloadButton, &KNS3::Button::dialogFinished, this, [this](const KNS3::Entry::List &changedEntries) {
        if (!changedEntries.isEmpty()) {
            m_pluginSelector->clearPlugins();
            m_pluginSelector->addPlugins(KPluginInfo::fromMetaData(Plasma::RunnerManager::runnerMetaDataList()),
                                         KPluginSelector::ReadConfigFile,
                                         i18n("Available Plugins"),
                                         QString(),
                                         m_config);
        }
    });
    downloadLayout->addStretch();
    downloadLayout->addWidget(downloadButton);
    layout->addLayout(downloadLayout);

    connect(this, &SearchConfigModule::defaultsIndicatorsVisibleChanged, this, &SearchConfigModule::updateUnmanagedState);
    connect(this, &SearchConfigModule::defaultsIndicatorsVisibleChanged, m_pluginSelector, &KPluginSelector::setDefaultsIndicatorsVisible);
    addConfig(m_settings, this);
}

void SearchConfigModule::load()
{
    KCModule::load();

    m_topPositioning->setChecked(!m_settings->freeFloating());
    m_freeFloating->setChecked(m_settings->freeFloating());

    m_clearHistoryButton->setEnabled(m_enableHistory->isChecked());

    // Set focus on the pluginselector to pass focus to search bar.
    m_pluginSelector->setFocus(Qt::OtherFocusReason);

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
    QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
    m_pluginSelector->addPlugins(KPluginInfo::fromMetaData(Plasma::RunnerManager::runnerMetaDataList()),
                                 KPluginSelector::ReadConfigFile,
                                 i18n("Available Plugins"),
                                 QString(),
                                 m_config);
    QT_WARNING_POP
    m_pluginSelector->load();

    if (!m_pluginID.isEmpty()) {
        m_pluginSelector->showConfiguration(m_pluginID);
    }
    configureClearHistoryButton();
}

void SearchConfigModule::save()
{
    m_settings->setFreeFloating(m_freeFloating->isChecked());
    m_settings->save();

    KCModule::save();

    // Combine & write history
    if (!m_activityAware->isChecked()) {
        if (!m_historyConfigGroup.hasKey(nullUuid)) {
            QStringList activities = m_consumer->activities();
            activities.removeOne(m_consumer->currentActivity());
            QStringList newHistory = m_historyConfigGroup.readEntry(m_consumer->currentActivity(), QStringList());
            for (const QString &activity : qAsConst(activities)) {
                newHistory.append(m_historyConfigGroup.readEntry(activity, QStringList()));
            }
            newHistory.removeDuplicates();
            m_historyConfigGroup.writeEntry(nullUuid, newHistory, KConfig::Notify);
            m_historyConfigGroup.sync();
        }
    }

    m_pluginSelector->save();

    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/krunnerrc"), QStringLiteral("org.kde.kconfig.notify"), QStringLiteral("ConfigChanged"));
    const QHash<QString, QByteArrayList> changes = {{QStringLiteral("Plugins"), {}}};
    message.setArguments({QVariant::fromValue(changes)});
    QDBusConnection::sessionBus().send(message);
}

void SearchConfigModule::defaults()
{
    KCModule::defaults();

    m_topPositioning->setChecked(!m_settings->defaultFreeFloatingValue());
    m_freeFloating->setChecked(m_settings->defaultFreeFloatingValue());

    m_pluginSelector->defaults();
}

void SearchConfigModule::configureClearHistoryButton()
{
    const QStringList activities = m_consumer->activities();
    const QStringList historyKeys = m_historyConfigGroup.keyList();
    if (m_activityAware->isChecked() && activities.length() > 1) {
        auto *installMenu = new QMenu(m_clearHistoryButton);
        QAction *all = installMenu->addAction(m_clearHistoryButton->icon(), i18nc("delete history for all activities", "For all activities"));
        installMenu->setEnabled(!historyKeys.isEmpty());
        connect(all, &QAction::triggered, this, &SearchConfigModule::deleteAllHistory);
        for (const auto &key : activities) {
            KActivities::Info info(key);
            QIcon icon;
            const QString iconStr = info.icon();
            if (iconStr.isEmpty()) {
                icon = m_clearHistoryButton->icon();
            } else if (QFileInfo::exists(iconStr)) {
                icon = QIcon(iconStr);
            } else {
                icon = QIcon::fromTheme(iconStr);
            }
            QAction *singleActivity = installMenu->addAction(icon, i18nc("delete history for this activity", "For activity \"%1\"", info.name()));
            singleActivity->setEnabled(historyKeys.contains(key)); // Otherwise there would be nothing to delete
            connect(singleActivity, &QAction::triggered, this, [this, key]() {
                deleteHistoryGroup(key);
            });
            installMenu->addAction(singleActivity);
            m_clearHistoryButton->setText(i18n("Clear History…"));
        }
        m_clearHistoryButton->setMenu(installMenu);
    } else {
        m_clearHistoryButton->setText(i18n("Clear History"));
        m_clearHistoryButton->setMenu(nullptr);
        m_clearHistoryButton->setEnabled(m_settings->historyEnabled() && !historyKeys.isEmpty());
    }
}

void SearchConfigModule::deleteHistoryGroup(const QString &key)
{
    m_historyConfigGroup.deleteEntry(key, KConfig::Notify);
    m_historyConfigGroup.sync();
    configureClearHistoryButton();
}

void SearchConfigModule::deleteAllHistory()
{
    m_historyConfigGroup.deleteGroup(KConfig::Notify);
    m_historyConfigGroup.sync();
    configureClearHistoryButton();
}

void SearchConfigModule::updateUnmanagedState()
{
    bool isNeedSave = false;
    isNeedSave |= m_pluginSelector->isSaveNeeded();
    isNeedSave |= m_topPositioning->isChecked() == m_settings->freeFloating();
    isNeedSave |= m_freeFloating->isChecked() != m_settings->freeFloating();

    unmanagedWidgetChangeState(isNeedSave);

    bool isDefault = true;
    isDefault &= m_pluginSelector->isDefault();
    isDefault &= m_topPositioning->isChecked() != m_settings->defaultFreeFloatingValue();
    isDefault &= m_freeFloating->isChecked() == m_settings->defaultFreeFloatingValue();

    setDefaultIndicatorVisible(m_topPositioning, defaultsIndicatorsVisible() && m_topPositioning->isChecked() == m_settings->defaultFreeFloatingValue());
    setDefaultIndicatorVisible(m_freeFloating, defaultsIndicatorsVisible() && m_freeFloating->isChecked() != m_settings->defaultFreeFloatingValue());

    unmanagedWidgetDefaultState(isDefault);
}

void SearchConfigModule::setDefaultIndicatorVisible(QWidget *widget, bool visible)
{
    widget->setProperty("_kde_highlight_neutral", visible);
    widget->update();
}

#include "kcm.moc"
