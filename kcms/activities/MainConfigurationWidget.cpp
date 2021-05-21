/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "MainConfigurationWidget.h"

#include <utils/d_ptr_implementation.h>

#include "ui_MainConfigurationWidgetBase.h"

#include "ActivitiesTab.h"
#include "PrivacyTab.h"
#include "SwitchingTab.h"
#include "kactivitiesdata.h"

K_PLUGIN_FACTORY(ActivitiesKCMFactory, registerPlugin<MainConfigurationWidget>(); registerPlugin<KActivitiesData>();)

class MainConfigurationWidget::Private : public Ui::MainConfigurationWidgetBase
{
public:
    ActivitiesTab *tabActivities;
    SwitchingTab *tabSwitching;
    PrivacyTab *tabPrivacy;
};

MainConfigurationWidget::MainConfigurationWidget(QWidget *parent, QVariantList args)
    : KCModule(parent, args)
    , d()
{
    d->setupUi(this);

    d->tabs->insertTab(0, d->tabActivities = new ActivitiesTab(d->tabs), i18n("Activities"));
    d->tabs->insertTab(1, d->tabSwitching = new SwitchingTab(d->tabs), i18n("Switching"));
    d->tabs->insertTab(2, d->tabPrivacy = new PrivacyTab(d->tabs), i18n("Privacy"));

    addConfig(d->tabPrivacy->pluginConfig(), d->tabPrivacy);
    addConfig(d->tabSwitching->mainConfig(), d->tabSwitching);

    connect(d->tabPrivacy, &PrivacyTab::blackListModelChanged, this, &MainConfigurationWidget::unmanagedWidgetChangeState);
    connect(d->tabPrivacy, &PrivacyTab::blackListModelDefaulted, this, &MainConfigurationWidget::unmanagedWidgetDefaultState);
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
