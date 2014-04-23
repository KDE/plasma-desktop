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

#include <QVBoxLayout>
#include <QLabel>

K_PLUGIN_FACTORY(SearchConfigModuleFactory, registerPlugin<SearchConfigModule>();)
K_EXPORT_PLUGIN(SearchConfigModuleFactory("kcm_search"))

SearchConfigModule::SearchConfigModule(QWidget* parent, const QVariantList& args)
    : KCModule(parent, args)
{
    KAboutData* about = new KAboutData(
        "kcm_search", "kcm_search", i18n("Configure Search"),
        "0.1", QString(), KAboutData::License_LGPL);
    about->addAuthor(i18n("Vishesh Handa"), QString(), "vhanda@kde.org");
    setAboutData(about);
    setButtons(Help | Apply | Default);

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* label = new QLabel(i18n("Select the search plugins"));
    m_listWidget = new QListWidget(this);
    connect(m_listWidget, SIGNAL(itemChanged(QListWidgetItem*)),
            this, SLOT(changed()));

    layout->addWidget(label);
    layout->addWidget(m_listWidget);
}


static int PluginNameRole = Qt::UserRole + 1;

void SearchConfigModule::load()
{
    m_listWidget->clear();

    KConfig config("krunnerrc");
    KConfigGroup group = config.group("Plugins");

    KPluginInfo::List list = Plasma::RunnerManager::listRunnerInfo();
    Q_FOREACH (const KPluginInfo& info, list) {
        QListWidgetItem* item = new QListWidgetItem(info.name());
        item->setIcon(QIcon::fromTheme(info.icon()));
        item->setData(PluginNameRole, info.pluginName());

        bool enabled = group.readEntry(info.pluginName() + "Enabled", true);
        item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);

        m_listWidget->addItem(item);
    }
}


void SearchConfigModule::save()
{
    KConfig config("krunnerrc");
    KConfigGroup group = config.group("Plugins");

    for (int i = 0; i < m_listWidget->count(); i++) {
        QListWidgetItem* item = m_listWidget->item(i);
        QString pluginName = item->data(PluginNameRole).toString();

        bool enabled = (item->checkState() == Qt::Checked);
        group.writeEntry(pluginName + "Enabled", enabled);
    }
}

void SearchConfigModule::defaults()
{
}

#include "kcm.moc"
