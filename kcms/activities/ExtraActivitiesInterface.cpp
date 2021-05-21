/*
    SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
