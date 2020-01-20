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

#include "SwitchingTab.h"
#include "kactivitymanagerd_settings.h"

#include <KActionCollection>
#include <KConfigGroup>
#include <KGlobalAccel>
#include <KSharedConfig>

#include "ui_SwitchingTabBase.h"

#include <utils/d_ptr_implementation.h>
#include <KActivities/Consumer>

class SwitchingTab::Private : public Ui::SwitchingTabBase {
public:
    KActivityManagerdSettings *mainConfig;

    KActionCollection *mainActionCollection;
    KActivities::Consumer activities;

    void createAction(const QString &actionName, const QString &actionText,
                      const QList<QKeySequence> &defaultSequence)
    {
        auto action = mainActionCollection->addAction(actionName);
        action->setProperty("isConfigurationAction", true);
        action->setText(actionText);
        KGlobalAccel::self()->setShortcut(action, defaultSequence);
        KGlobalAccel::self()->setDefaultShortcut(action, defaultSequence);
    }

    Private()
        : mainConfig(new KActivityManagerdSettings),
          mainActionCollection(nullptr)
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

    d->createAction(QStringLiteral("next activity"),
                    i18nc("@action", "Walk through activities"),
                    { Qt::META + Qt::Key_Tab });
    d->createAction(QStringLiteral("previous activity"),
                    i18nc("@action", "Walk through activities (Reverse)"),
                    { Qt::META + Qt::SHIFT + Qt::Key_Tab } );

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
    QString actionName = sender()
                             ? sender()->property("shortcutAction").toString()
                             : QString();

    if (actionName.isEmpty()) return;

    auto action = d->mainActionCollection->action(actionName);

    KGlobalAccel::self()->setShortcut(action, { sequence },
                                      KGlobalAccel::NoAutoloading);
    d->mainActionCollection->writeSettings();
}

