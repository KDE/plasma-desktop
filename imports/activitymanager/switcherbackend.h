/*
    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SWITCHER_BACKEND_H
#define SWITCHER_BACKEND_H

// Qt
#include <QHash>
#include <QJSValue>
#include <QKeySequence>
#include <QObject>
#include <QPixmap>
#include <QTimer>

// KDE
#include <KActivities/Controller>

// Local
#include "sortedactivitiesmodel.h"

class QAction;
class QRasterWindow;
class QQmlEngine;
class QJSEngine;

namespace KIO
{
class PreviewJob;
}

class SwitcherBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool shouldShowSwitcher READ shouldShowSwitcher WRITE setShouldShowSwitcher NOTIFY shouldShowSwitcherChanged)
    Q_PROPERTY(bool dropEnabled READ dropEnabled CONSTANT)

public:
    explicit SwitcherBackend(QObject *parent = nullptr);
    ~SwitcherBackend() override;

    static QObject *instance(QQmlEngine *engine, QJSEngine *scriptEngine);

Q_SIGNALS:
    void showSwitchNotification(const QString &id, const QString &name, const QString &icon);
    void shouldShowSwitcherChanged(bool value);

public Q_SLOTS:
    void init();

    bool shouldShowSwitcher() const;
    void setShouldShowSwitcher(bool shouldShowSwitcher);

    QAbstractItemModel *runningActivitiesModel() const;
    QAbstractItemModel *stoppedActivitiesModel() const;

    void setCurrentActivity(const QString &activity);
    void stopActivity(const QString &activity);

    void setDropMode(bool value);
    void drop(QMimeData *mimeData, int modifiers, const QVariant &activityId);
    void dropCopy(QMimeData *mimeData, const QVariant &activityId);
    void dropMove(QMimeData *mimeData, const QVariant &activityId);
    bool dropEnabled() const;

    void toggleActivityManager();

private:
    template<typename Handler>
    inline void registerShortcut(const QString &actionName, const QString &name, const QKeySequence &shortcut, Handler &&handler);

    enum Direction {
        Next,
        Previous,
    };

    void switchToActivity(Direction i);

private Q_SLOTS:
    void keybdSwitchToNextActivity();
    void keybdSwitchToPreviousActivity();
    void keybdSwitchedToAnotherActivity();

    void showActivitySwitcherIfNeeded();

    void onCurrentActivityChanged(const QString &id);

private:
    QHash<QString, QKeySequence> m_actionShortcut;
    QAction *m_lastInvokedAction = nullptr;
    QRasterWindow *m_inputWindow = nullptr;
    KActivities::Controller m_activities;
    bool m_shouldShowSwitcher;
    QTimer m_modKeyPollingTimer;
    QString m_previousActivity;

    bool m_dropModeActive;
    QTimer m_dropModeHider;

    SortedActivitiesModel *m_runningActivitiesModel = nullptr;
    SortedActivitiesModel *m_stoppedActivitiesModel = nullptr;
};

#endif // SWITCHER_BACKEND_H
