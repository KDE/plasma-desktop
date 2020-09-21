/* This file is part of the KDE Project
   Copyright (c) 2014 Vishesh Handa <me@vhanda.in>
   Copyright (c) 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

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

#include "kcm.h"

#include <KPluginFactory>
#include <KAboutData>
#include <KSharedConfig>
#include <QDebug>
#include <QStandardPaths>
#include <KLocalizedString>
#include <KRunner/RunnerManager>
#include <KCModuleInfo>
#include <KCModuleProxy>
#include <KPluginSelector>

#include <QApplication>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusMetaType>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialog>
#include <QPainter>
#include <QPushButton>
#include <QFormLayout>

K_PLUGIN_FACTORY(SearchConfigModuleFactory, registerPlugin<SearchConfigModule>();)


SearchConfigModule::SearchConfigModule(QWidget* parent, const QVariantList& args)
    : KCModule(parent, args)
    , m_config("krunnerrc")
{
    KAboutData* about = new KAboutData(QStringLiteral("kcm_search"), i18nc("kcm name for About dialog", "Configure Search Bar"),
                                       QStringLiteral("0.1"), QString(), KAboutLicense::LGPL);
    about->addAuthor(i18n("Vishesh Handa"), QString(), QStringLiteral("vhanda@kde.org"));
    setAboutData(about);
    setButtons(Apply | Default);

    if(!args.at(0).toString().isEmpty()) {
        m_pluginID = args.at(0).toString();
    }

    QVBoxLayout* layout = new QVBoxLayout(this);

    QHBoxLayout *headerLayout = new QHBoxLayout(this);

    QLabel *label = new QLabel(i18n("Enable or disable KRunner plugins:"));

    m_clearHistoryButton = new QPushButton(i18n("Clear History"));
    m_clearHistoryButton->setIcon(QIcon::fromTheme(isRightToLeft() ? QStringLiteral("edit-clear-locationbar-ltr")
                                                                   : QStringLiteral("edit-clear-locationbar-rtl")));
    connect(m_clearHistoryButton, &QPushButton::clicked, this, [this] {
        KConfigGroup generalConfig(m_config.group("General"));
        generalConfig.deleteEntry("history", KConfig::Notify);
        generalConfig.sync();
    });

    QHBoxLayout *configHeaderLayout = new QHBoxLayout(this);
    QVBoxLayout *configHeaderLeft = new QVBoxLayout(this);
    QVBoxLayout *configHeaderRight = new QVBoxLayout(this);

    // Options where KRunner should pop up
    m_topPositioning = new QRadioButton(i18n("Top"), this);
    connect(m_topPositioning, &QRadioButton::clicked, this, &SearchConfigModule::markAsChanged);
    m_freeFloating = new QRadioButton(i18n("Center"), this);
    connect(m_freeFloating, &QRadioButton::clicked, this, &SearchConfigModule::markAsChanged);

    QFormLayout *positionLayout = new QFormLayout(this);
    positionLayout->addRow(i18n("Position on screen:"), m_topPositioning);
    positionLayout->addRow(QString(), m_freeFloating);
    configHeaderLeft->addLayout(positionLayout);
    m_enableHistory = new QCheckBox(i18n("Enable"), this);
    positionLayout->addItem(new QSpacerItem(0, 0));
    positionLayout->addRow(i18n("History:"), m_enableHistory);
    connect(m_enableHistory, &QCheckBox::clicked, this, &SearchConfigModule::markAsChanged);
    connect(m_enableHistory, &QCheckBox::clicked, m_clearHistoryButton, &QPushButton::setEnabled);
    m_retainPriorSearch = new QCheckBox(i18n("Retain previous search"), this);
    connect(m_retainPriorSearch, &QCheckBox::clicked, this, &SearchConfigModule::markAsChanged);
    positionLayout->addRow(QString(), m_retainPriorSearch);
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

    connect(m_pluginSelector, &KPluginSelector::changed, this, [this] { markAsChanged(); });
    connect(m_pluginSelector, &KPluginSelector::defaulted, this, &KCModule::defaulted);

    qDBusRegisterMetaType<QByteArrayList>();
    qDBusRegisterMetaType<QHash<QString, QByteArrayList>>();
    // This will trigger the reloadConfiguration method for the runner
    connect(m_pluginSelector, &KPluginSelector::configCommitted, this, [](const QByteArray &componentName){
        QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/krunnerrc"),
                                                          QStringLiteral("org.kde.kconfig.notify"),
                                                          QStringLiteral("ConfigChanged"));
        const QHash<QString, QByteArrayList> changes = {{QStringLiteral("Runners"), {componentName}}};
        message.setArguments({QVariant::fromValue(changes)});
        QDBusConnection::sessionBus().send(message);
    });

    layout->addLayout(configHeaderLayout);
    layout->addSpacing(12);
    layout->addLayout(headerLayout);
    layout->addWidget(m_pluginSelector);
}

void SearchConfigModule::load()
{
    KSharedConfigPtr config = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    const KConfigGroup general = config->group("General");
    bool freeFloating = general.readEntry("FreeFloating", false);
    m_topPositioning->setChecked(!freeFloating);
    m_freeFloating->setChecked(freeFloating);
    m_retainPriorSearch->setChecked(general.readEntry("RetainPriorSearch", true));
    bool historyEnabled = general.readEntry("HistoryEnabled", true);
    m_enableHistory->setChecked(historyEnabled);
    m_clearHistoryButton->setEnabled(historyEnabled);

    // Set focus on the pluginselector to pass focus to search bar.
    m_pluginSelector->setFocus(Qt::OtherFocusReason);

    m_pluginSelector->addPlugins(Plasma::RunnerManager::listRunnerInfo(),
                    KPluginSelector::ReadConfigFile,
                    i18n("Available Plugins"), QString(),
                    config);
    m_pluginSelector->load();

    if(!m_pluginID.isEmpty()){
        m_pluginSelector->showConfiguration(m_pluginID);
    }
}


void SearchConfigModule::save()
{
    KSharedConfigPtr config = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    config->group("General").writeEntry("FreeFloating", m_freeFloating->isChecked(), KConfig::Notify);
    config->group("General").writeEntry("RetainPriorSearch", m_retainPriorSearch->isChecked(), KConfig::Notify);
    config->group("General").writeEntry("HistoryEnabled", m_enableHistory->isChecked(), KConfig::Notify);
    m_pluginSelector->save();

    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/krunnerrc"),
                                                      QStringLiteral("org.kde.kconfig.notify"),
                                                      QStringLiteral("ConfigChanged"));
    const QHash<QString, QByteArrayList> changes = {{QStringLiteral("Plugins"), {}}};
    message.setArguments({QVariant::fromValue(changes)});
    QDBusConnection::sessionBus().send(message);
}

void SearchConfigModule::defaults()
{
    m_topPositioning->setChecked(true);
    m_freeFloating->setChecked(false);
    m_retainPriorSearch->setChecked(true);
    m_enableHistory->setChecked(true);
    m_clearHistoryButton->setEnabled(true);
    m_pluginSelector->defaults();
}

#include "kcm.moc"
