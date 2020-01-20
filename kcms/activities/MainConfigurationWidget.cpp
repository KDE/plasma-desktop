/*
 *   Copyright (C) 2012 - 2016 by Ivan Cukic <ivan.cukic@kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MainConfigurationWidget.h"

#include <utils/d_ptr_implementation.h>

#include "ui_MainConfigurationWidgetBase.h"

#include "ActivitiesTab.h"
#include "SwitchingTab.h"
#include "PrivacyTab.h"

K_PLUGIN_FACTORY(ActivitiesKCMFactory, registerPlugin<MainConfigurationWidget>();)

class MainConfigurationWidget::Private : public Ui::MainConfigurationWidgetBase {
public:
    ActivitiesTab * tabActivities;
    SwitchingTab * tabSwitching;
    PrivacyTab * tabPrivacy;
};

MainConfigurationWidget::MainConfigurationWidget(QWidget *parent, QVariantList args)
    : KCModule(parent, args)
    , d()
{
    d->setupUi(this);

    d->tabs->insertTab(0, d->tabActivities = new ActivitiesTab(d->tabs), i18n("Activities"));
    d->tabs->insertTab(1, d->tabSwitching  = new SwitchingTab(d->tabs), i18n("Switching"));
    d->tabs->insertTab(2, d->tabPrivacy    = new PrivacyTab(d->tabs), i18n("Privacy"));
    
    addConfig(d->tabPrivacy->pluginConfig(), d->tabPrivacy);
    addConfig(d->tabSwitching->mainConfig(), d->tabSwitching);
    
    connect (d->tabPrivacy, &PrivacyTab::blackListModelChanged, this, &MainConfigurationWidget::unmanagedWidgetChangeState);
    connect (d->tabPrivacy, &PrivacyTab::blackListModelDefaulted, this, &MainConfigurationWidget::unmanagedWidgetDefaultState);
}

MainConfigurationWidget::~MainConfigurationWidget()
{
}

void MainConfigurationWidget::defaults()
{
    KCModule::defaults();
    
    d->tabPrivacy->defaults();
}

void MainConfigurationWidget::load()
{
    KCModule::load();
    
    d->tabPrivacy->load();
}

void MainConfigurationWidget::save()
{
    KCModule::save();
    
    d->tabPrivacy->save();
}

#include "MainConfigurationWidget.moc"
