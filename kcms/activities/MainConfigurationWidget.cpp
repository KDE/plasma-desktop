/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "MainConfigurationWidget.h"

#include <utils/d_ptr_implementation.h>

#include "ui_MainConfigurationWidgetBase.h"

#include "ActivitiesTab.h"
#include "SwitchingTab.h"
#include "kactivitiesdata.h"

K_PLUGIN_FACTORY_WITH_JSON(ActivitiesKCMFactory, "kcm_activities.json", registerPlugin<MainConfigurationWidget>(); registerPlugin<KActivitiesData>();)

class MainConfigurationWidget::Private : public Ui::MainConfigurationWidgetBase
{
public:
    ActivitiesTab *tabActivities;
    SwitchingTab *tabSwitching;
};

MainConfigurationWidget::MainConfigurationWidget(QWidget *parent, QVariantList args)
    : KCModule(parent, args)
    , d()
{
    d->setupUi(this);

    d->tabs->insertTab(0, d->tabActivities = new ActivitiesTab(d->tabs), i18n("Activities"));
    d->tabs->insertTab(1, d->tabSwitching = new SwitchingTab(d->tabs), i18n("Switching"));

    addConfig(d->tabSwitching->mainConfig(), d->tabSwitching);
}

MainConfigurationWidget::~MainConfigurationWidget()
{
}

void MainConfigurationWidget::defaults()
{
    KCModule::defaults();
}

void MainConfigurationWidget::load()
{
    KCModule::load();
}

void MainConfigurationWidget::save()
{
    KCModule::save();
}

#include "MainConfigurationWidget.moc"
