/*
 *  SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>
 *  SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "activityconfig.h"

#include "utils/continue_with.h"

#include <PlasmaActivities/Info>

#include <KConfig>
#include <KConfigGroup>
#include <KGlobalAccel>
#include <KLocalizedString>

#include <QAction>
#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

ActivityConfig::ActivityConfig(QObject *parent)
    : QObject(parent)
{
    reset();
}

ActivityConfig::~ActivityConfig()
{
}

void ActivityConfig::reset()
{
    m_name = QString();
    m_description = QString();
    m_iconName = QStringLiteral("activities");
    m_private = false;
    m_shortcut = QKeySequence();
    m_inhibitScreen = false;
    m_inhibitSleep = false;
}

QString ActivityConfig::activityId() const
{
    return m_activityId;
}

void ActivityConfig::setActivityId(const QString &activityId)
{
    if (activityId == m_activityId) {
        return;
    }

    m_activityId = activityId;
    Q_EMIT activityIdChanged();

    load();
}

bool ActivityConfig::isSaveNeeded()
{
    KActivities::Info activityInfo(m_activityId);
    if (activityInfo.name() != m_name || activityInfo.description() != m_description || activityInfo.icon() != m_iconName) {
        return true;
    }

    const auto shortcuts = KGlobalAccel::self()->globalShortcut(QStringLiteral("ActivityManager"), QStringLiteral("switch-to-activity-%1").arg(m_activityId));
    QKeySequence savedShortcut = shortcuts.isEmpty() ? QKeySequence() : shortcuts.first();
    if (savedShortcut != m_shortcut) {
        return true;
    }

    // The private state is retrieved/set via DBus. We store it in a member variable for convenience
    if (m_private != m_savedPrivate) {
        return true;
    }

    if (m_inhibitScreen != m_savedInhibitScreen) {
        return true;
    }

    if (m_inhibitSleep != m_savedInhibitSleep) {
        return true;
    }

    return false;
}

void ActivityConfig::load()
{
    if (m_activityId.isEmpty()) {
        reset();
        Q_EMIT infoChanged();
        return;
    }

    KActivities::Info activityInfo(m_activityId);
    m_name = activityInfo.name();
    m_description = activityInfo.description();
    m_iconName = activityInfo.icon();

    // finding the key shortcut
    const auto shortcuts = KGlobalAccel::self()->globalShortcut(QStringLiteral("ActivityManager"), QStringLiteral("switch-to-activity-%1").arg(m_activityId));
    m_shortcut = shortcuts.isEmpty() ? QKeySequence() : shortcuts.first();

    // Get "isPrivate" property via DBus
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.ActivityManager"),
                                                          QStringLiteral("/ActivityManager/Features"),
                                                          QStringLiteral("org.kde.ActivityManager.Features"),
                                                          QStringLiteral("GetValue"));
    message.setArguments({QStringLiteral("org.kde.ActivityManager.Resources.Scoring/isOTR/%1").arg(m_activityId)});

    QDBusPendingCall result = QDBusConnection::sessionBus().asyncCall(message);
    auto watcher = new QDBusPendingCallWatcher(result, this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [&](QDBusPendingCallWatcher *watcher) mutable {
        QDBusPendingReply<QDBusVariant> reply = *watcher;
        m_private = false;
        if (!reply.isError()) {
            m_private = reply.value().variant().toBool();
        }
        m_savedPrivate = m_private;
        Q_EMIT infoChanged();
        watcher->deleteLater();
    });

    KConfig powerdevilConfig("powerdevilrc");
    KConfigGroup activityGroup = powerdevilConfig.group(QStringLiteral("Activities")).group(m_activityId);
    m_inhibitScreen = activityGroup.readEntry("InhibitScreenManagement", false);
    m_savedInhibitScreen = m_inhibitScreen;
    m_inhibitSleep = activityGroup.readEntry("InhibitSuspend", false);
    m_savedInhibitSleep = m_inhibitSleep;

    Q_EMIT infoChanged();
}

void ActivityConfig::save()
{
    if (m_activityId.isEmpty()) {
        createActivity();
        return;
    }

    m_activities.setActivityName(m_activityId, m_name);
    m_activities.setActivityDescription(m_activityId, m_description);
    m_activities.setActivityIcon(m_activityId, m_iconName);

    // setting the key shortcut
    QAction action(nullptr);
    action.setProperty("isConfigurationAction", true);
    action.setProperty("componentName", QStringLiteral("ActivityManager"));
    action.setObjectName(QStringLiteral("switch-to-activity-%1").arg(m_activityId));
    KGlobalAccel::self()->setShortcut(&action, {m_shortcut}, KGlobalAccel::NoAutoloading);

    // Set "isPrivate" property via DBus
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.ActivityManager"),
                                                          QStringLiteral("/ActivityManager/Features"),
                                                          QStringLiteral("org.kde.ActivityManager.Features"),
                                                          QStringLiteral("SetValue"));
    message.setArguments({
        QStringLiteral("org.kde.ActivityManager.Resources.Scoring/isOTR/%1").arg(m_activityId),
        QVariant::fromValue(QDBusVariant(m_private)),
    });
    QDBusConnection::sessionBus().asyncCall(message);

    KConfig powerdevilConfig("powerdevilrc");
    KConfigGroup activityGroup = powerdevilConfig.group(QStringLiteral("Activities")).group(m_activityId);
    activityGroup.writeEntry("InhibitScreenManagement", m_inhibitScreen);
    m_savedInhibitScreen = m_inhibitScreen;
    activityGroup.writeEntry("InhibitSuspend", m_inhibitSleep);
    m_savedInhibitSleep = m_inhibitSleep;
}

void ActivityConfig::createActivity()
{
    using namespace kamd::utils;
    continue_with(KActivities::Controller().addActivity(m_name), [this](const optional_view<QString> &activityId) {
        if (activityId.is_initialized()) {
            m_activityId = activityId.get();
            save();
        }
    });
}

#include "moc_activityconfig.cpp"
