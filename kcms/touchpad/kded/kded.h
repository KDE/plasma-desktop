/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QDBusPendingCallWatcher>
#include <QDBusServiceWatcher>
#include <QPointer>
#include <QTimer>
#include <QVariantList>

#include <KDEDModule>

#include "touchpadbackend.h"

class TouchpadDisabler : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.touchpad")

public:
    TouchpadDisabler(QObject *, const QVariantList &);

Q_SIGNALS:
    Q_SCRIPTABLE void enabledChanged(bool);
    Q_SCRIPTABLE void workingTouchpadFoundChanged(bool);

public Q_SLOTS:
    Q_SCRIPTABLE Q_NOREPLY void toggle();
    Q_SCRIPTABLE Q_NOREPLY void disable();
    Q_SCRIPTABLE Q_NOREPLY void enable();
    Q_SCRIPTABLE bool isEnabled() const;
    Q_SCRIPTABLE bool workingTouchpadFound() const;

private Q_SLOTS:
    void updateCurrentState();
    void serviceRegistered(const QString &);
    void handleReset();
    void serviceNameFetchFinished(QDBusPendingCallWatcher *);
    void onPrepareForSleep(bool sleep);

private:
    void lateInit();
    void updateWorkingTouchpadFound();
    void showOsd();

    TouchpadBackend *m_backend;
    QDBusServiceWatcher m_dependencies;

    bool m_userRequestedState, m_touchpadEnabled;
    bool m_workingTouchpadFound;

    bool m_preparingForSleep = false;
};
