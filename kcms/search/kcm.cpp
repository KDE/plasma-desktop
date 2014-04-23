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

SearchConfigModule::SearchConfigModule(QWidget* parent, const QVariantList& args)
    : KCModule(parent, args)
{
    KAboutData* about = new KAboutData("kcm_search", i18n("Configure Search"),
                                       "0.1", QString(), KAboutLicense::LGPL);
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

    m_manager = new Plasma::RunnerManager(this);
}


void SearchConfigModule::load()
{
    m_listWidget->clear();

    QStringList disabledCategories = m_manager->disabledCategories();
    KPluginInfo::List list = Plasma::RunnerManager::listRunnerInfo();
    Q_FOREACH (const KPluginInfo& info, list) {
        QString runnerName = info.name();

        Plasma::AbstractRunner* runner = info.service()->createInstance<Plasma::AbstractRunner>();
        if (!runner)
            continue;

        QStringList categories = runner->categories();
        Q_FOREACH (const QString& category, categories) {
            QListWidgetItem* item = new QListWidgetItem(category);
            item->setIcon(QIcon::fromTheme(info.icon()));

            bool disabled = disabledCategories.contains(category);
            item->setCheckState(disabled ? Qt::Unchecked : Qt::Checked);

            m_listWidget->addItem(item);
        }

        delete runner;
    }
}


void SearchConfigModule::save()
{
    QStringList disabledCategories;
    for (int i = 0; i < m_listWidget->count(); i++) {
        QListWidgetItem* item = m_listWidget->item(i);
        QString category = item->text();

        bool enabled = (item->checkState() == Qt::Checked);
        if (!enabled) {
            disabledCategories << category;
        }
    }

    m_manager->setDisabledCategories(disabledCategories);
}

void SearchConfigModule::defaults()
{
}

#include "kcm.moc"
