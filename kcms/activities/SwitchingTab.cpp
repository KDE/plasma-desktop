/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "SwitchingTab.h"
#include "kactivitymanagerd_settings.h"

#include <KActionCollection>
#include <KConfigGroup>
#include <KGlobalAccel>
#include <KSharedConfig>

#include "ui_SwitchingTabBase.h"

#include <KActivities/Consumer>
#include <utils/d_ptr_implementation.h>

class SwitchingTab::Private : public Ui::SwitchingTabBase
{
public:
    KActivityManagerdSettings *mainConfig;

    KActionCollection *mainActionCollection;
    KActivities::Consumer activities;

    void createAction(const QString &actionName, const QString &actionText, const QList<QKeySequence> &defaultSequence)
    {
        auto action = mainActionCollection->addAction(actionName);
        action->setProperty("isConfigurationAction", true);
        action->setText(actionText);
        KGlobalAccel::self()->setShortcut(action, defaultSequence);
        KGlobalAccel::self()->setDefaultShortcut(action, defaultSequence);
    }

    Private()
        : mainConfig(new KActivityManagerdSettings)
        , mainActionCollection(nullptr)
    {
    }
};

SwitchingTab::SwitchingTab(QWidget *parent)
    : QWidget(parent)
    , d()
{
    d->setupUi(this);

    // Shortcut config. The shortcut belongs to the component "plasmashell"!
    d->mainActionCollection = new KActionCollection(this, QStringLiteral("plasmashell"));
    d->mainActionCollection->setComponentDisplayName(i18n("Activity switching"));
    d->mainActionCollection->setConfigGlobal(true);

    d->createAction(QStringLiteral("next activity"), i18nc("@action", "Walk through activities"), {Qt::META | Qt::Key_Tab});
    d->createAction(QStringLiteral("previous activity"), i18nc("@action", "Walk through activities (Reverse)"), {Qt::META | Qt::SHIFT | Qt::Key_Tab});

    d->scActivities->setActionTypes(KShortcutsEditor::GlobalAction);
    d->scActivities->addCollection(d->mainActionCollection);
}

SwitchingTab::~SwitchingTab()
{
}

KCoreConfigSkeleton *SwitchingTab::mainConfig()
{
    return d->mainConfig;
}

void SwitchingTab::shortcutChanged(const QKeySequence &sequence)
{
    QString actionName = sender() ? sender()->property("shortcutAction").toString() : QString();

    if (actionName.isEmpty())
        return;

    auto action = d->mainActionCollection->action(actionName);

    KGlobalAccel::self()->setShortcut(action, {sequence}, KGlobalAccel::NoAutoloading);
    d->mainActionCollection->writeSettings();
}
