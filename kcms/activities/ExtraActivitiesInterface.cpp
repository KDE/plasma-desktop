/*
 *   Copyright (C) 2015 - 2016 by Ivan Cukic <ivan.cukic@kde.org>
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

#include "ExtraActivitiesInterface.h"

#include <KActionCollection>
#include <KActivities/Info>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <QAction>

#include <memory>
#include <utils/d_ptr_implementation.h>

#include "common/dbus/common.h"
#include "features_interface.h"

#define ENABLE_QJSVALUE_CONTINUATION
#include "utils/continue_with.h"

class ExtraActivitiesInterface::Private
{
public:
    Private(ExtraActivitiesInterface *q)
        : features(new org::kde::ActivityManager::Features(QStringLiteral(KAMD_DBUS_SERVICE),
                                                           QStringLiteral(KAMD_DBUS_FEATURES_PATH),
                                                           QDBusConnection::sessionBus(),
                                                           q))
        , activitiesActionCollection(new KActionCollection(q, QStringLiteral("ActivityManager")))
    {
        activitiesActionCollection->setComponentDisplayName(i18n("Activities"));
        activitiesActionCollection->setConfigGlobal(true);
    }

    ~Private()
    {
    }

    QAction *actionForActivity(const QString &activity)
    {
        if (!activityActions.contains(activity)) {
            auto action = activitiesActionCollection->addAction(QStringLiteral("switch-to-activity-") + activity);

            activityActions[activity] = action;
            action->setProperty("isConfigurationAction", true);

            KGlobalAccel::self()->setShortcut(action, {});
        }

        return activityActions[activity];
    }

    std::unique_ptr<org::kde::ActivityManager::Features> features;
    std::unique_ptr<KActionCollection> activitiesActionCollection;
    QHash<QString, QAction *> activityActions;
};

ExtraActivitiesInterface::ExtraActivitiesInterface(QObject *parent)
    : QObject(parent)
    , d(this)
{
}

ExtraActivitiesInterface::~ExtraActivitiesInterface()
{
}

void ExtraActivitiesInterface::setIsPrivate(const QString &activity, bool isPrivate, QJSValue callback)
{
    auto result = d->features->SetValue(QStringLiteral("org.kde.ActivityManager.Resources.Scoring/isOTR/") + activity, QDBusVariant(isPrivate));

    auto *watcher = new QDBusPendingCallWatcher(result, this);

    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [callback](QDBusPendingCallWatcher *watcher) mutable {
        callback.call();
        watcher->deleteLater();
    });
}

void ExtraActivitiesInterface::getIsPrivate(const QString &activity, QJSValue callback)
{
    auto result = d->features->GetValue(QStringLiteral("org.kde.ActivityManager.Resources.Scoring/isOTR/") + activity);

    auto *watcher = new QDBusPendingCallWatcher(result, this);

    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [callback, result](QDBusPendingCallWatcher *watcher) mutable {
        QDBusPendingReply<QDBusVariant> reply = *watcher;
        callback.call({reply.value().variant().toBool()});
        watcher->deleteLater();
    });
}

void ExtraActivitiesInterface::setShortcut(const QString &activity, const QKeySequence &keySequence)
{
    auto action = d->actionForActivity(activity);

    KGlobalAccel::self()->setShortcut(action, {keySequence}, KGlobalAccel::NoAutoloading);
}

QKeySequence ExtraActivitiesInterface::shortcut(const QString &activity)
{
    auto action = d->actionForActivity(activity);

    const auto shortcuts = KGlobalAccel::self()->shortcut(action);
    return (shortcuts.isEmpty()) ? QKeySequence() : shortcuts.first();
}
