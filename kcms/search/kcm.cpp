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

#include <QVBoxLayout>
#include <QLabel>

K_PLUGIN_FACTORY(SearchConfigModuleFactory, registerPlugin<SearchConfigModule>();)

SearchConfigModule::SearchConfigModule(QWidget* parent, const QVariantList& args)
    : KCModule(parent, args)
    , m_config("krunnerrc")
    , m_configGroup(m_config.group("PlasmaRunnerManager"))
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
}


void SearchConfigModule::load()
{
    m_listWidget->clear();

    QStringList enabledCategories = m_configGroup.readEntry("enabledCategories", QStringList());

    KPluginInfo::List list = Plasma::RunnerManager::listRunnerInfo();
    Q_FOREACH (const KPluginInfo& info, list) {
        QVariantList args;
        args << info.service()->storageId();

        Plasma::AbstractRunner* r = info.service()->createInstance<Plasma::AbstractRunner>(this, args);
        QScopedPointer<Plasma::AbstractRunner> runner(r);
        if (runner.isNull())
            continue;

        QStringList categories = runner->categories();
        QString name = runner->name();

        Q_FOREACH (const QString& category, categories) {
            QListWidgetItem* item = new QListWidgetItem(category);
            item->setIcon(runner->categoryIcon(category));

            bool enabled = enabledCategories.isEmpty() || enabledCategories.contains(category);
            item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);

            m_listWidget->addItem(item);
        }
    }
}


void SearchConfigModule::save()
{
    QStringList enabledCategories;
    for (int i = 0; i < m_listWidget->count(); i++) {
        QListWidgetItem* item = m_listWidget->item(i);
        QString category = item->text();

        if (item->checkState() == Qt::Checked) {
            enabledCategories << category;
        }
    }

    m_configGroup.writeEntry("enabledCategories", enabledCategories);
    m_configGroup.sync();
}

void SearchConfigModule::defaults()
{
}

#include "kcm.moc"
