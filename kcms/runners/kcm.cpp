/* This file is part of the KDE Project
   Copyright (c) 2014 Vishesh Handa <me@vhanda.in>

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
#include <KPluginLoader>
#include <KAboutData>
#include <KSharedConfig>
#include <QDebug>
#include <QStandardPaths>
#include <KLocalizedString>
#include <KRunner/RunnerManager>
#include <KCModuleInfo>
#include <KCModuleProxy>
#include <KIconLoader>
#include <KPluginSelector>

#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPainter>
#include <QPushButton>

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

    QVBoxLayout* layout = new QVBoxLayout(this);

    QHBoxLayout *headerLayout = new QHBoxLayout(this);

    QLabel *label = new QLabel(i18n("Select the search plugins:"));

    QPushButton *clearHistoryButton = new QPushButton(i18n("Clear History"));
    clearHistoryButton->setIcon(QIcon::fromTheme(isRightToLeft() ? QStringLiteral("edit-clear-locationbar-ltr")
                                                                 : QStringLiteral("edit-clear-locationbar-rtl")));
    connect(clearHistoryButton, &QPushButton::clicked, this, [this] {
        KConfigGroup generalConfig(m_config.group("General"));
        generalConfig.deleteEntry("history");
        generalConfig.sync();
    });

    headerLayout->addWidget(label);
    headerLayout->addStretch();
    headerLayout->addWidget(clearHistoryButton);

    m_pluginSelector = new KPluginSelector(this);

    //overload, can't use the new syntax
    connect(m_pluginSelector, SIGNAL(changed(bool)),
            this, SIGNAL(changed(bool)));

    layout->addLayout(headerLayout);
    layout->addWidget(m_pluginSelector);

    Plasma::RunnerManager *manager = new Plasma::RunnerManager(this);
    manager->reloadConfiguration();
}

void SearchConfigModule::load()
{
    // Set focus on the pluginselector to pass focus to search bar.
    m_pluginSelector->setFocus(Qt::OtherFocusReason);

    m_pluginSelector->addPlugins(Plasma::RunnerManager::listRunnerInfo(),
                    KPluginSelector::ReadConfigFile,
                    i18n("Available Plugins"), QString(),
                    KSharedConfig::openConfig(QLatin1String( "krunnerrc" )));
}


void SearchConfigModule::save()
{
    m_pluginSelector->save();
}

void SearchConfigModule::defaults()
{
}

#include "kcm.moc"
