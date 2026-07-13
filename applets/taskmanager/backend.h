/*
    SPDX-FileCopyrightText: 2013-2016 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KConfigWatcher>

#include <QObject>
#include <QRect>

#include <netwm.h>
#include <qqmlregistration.h>
#include <qwindowdefs.h>

#include "kactivitymanagerd_plugins_settings.h"

class KActionCollection;
class QAction;
class QActionGroup;
class QQuickItem;
class QQuickWindow;
class QJsonArray;

namespace KActivities
{
class Consumer;
}

class Backend : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum MiddleClickAction {
        None = 0,
        Close,
        NewInstance,
        ToggleMinimized,
        ToggleGrouping,
        BringToCurrentDesktop,
    };

    Q_ENUM(MiddleClickAction)

    Q_PROPERTY(int screen READ screen WRITE setScreen NOTIFY screenChanged)
    Q_PROPERTY(bool hasActiveTask READ hasActiveTask WRITE setHasActiveTask NOTIFY hasActiveTaskChanged)
    Q_PROPERTY(bool showsOnlyCurrentScreen READ showsOnlyCurrentScreen WRITE setShowsOnlyCurrentScreen NOTIFY showsOnlyCurrentScreenChanged)

    explicit Backend(QObject *parent = nullptr);
    ~Backend() override;

    Q_INVOKABLE QVariantList jumpListActions(const QUrl &launcherUrl, QObject *parent);
    Q_INVOKABLE QVariantList placesActions(const QUrl &launcherUrl, bool showAllPlaces, QObject *parent);
    Q_INVOKABLE QVariantList recentDocumentActions(const QUrl &launcherUrl, QObject *parent);
    Q_INVOKABLE void setActionGroup(QAction *action) const;

    Q_INVOKABLE QRect globalRect(QQuickItem *item) const;

    Q_INVOKABLE bool isApplication(const QUrl &url) const;

    Q_INVOKABLE qint64 parentPid(qint64 pid) const;

    Q_INVOKABLE static QUrl tryDecodeApplicationsUrl(const QUrl &launcherUrl);
    Q_INVOKABLE static QStringList applicationCategories(const QUrl &launcherUrl);

    int screen() const;
    void setScreen(int screen);

    bool hasActiveTask() const;
    void setHasActiveTask(bool active);

    bool showsOnlyCurrentScreen() const;
    void setShowsOnlyCurrentScreen(bool shows);

Q_SIGNALS:
    void addLauncher(const QUrl &url) const;

    void showAllPlaces();

    void screenChanged();
    void hasActiveTaskChanged();
    void showsOnlyCurrentScreenChanged();

    void activateTaskAtIndexRequested(int index);
    void activatePreviousTaskRequested();
    void activateNextTaskRequested();
    void moveActiveTaskBackwardRequested();
    void moveActiveTaskForwardRequested();

private Q_SLOTS:
    void handleRecentDocumentAction() const;

private:
    static void setupShortcuts();
    static Backend *findTargetBackend();
    static void dispatchActivateTaskAtIndex(int index);
    static void dispatchActivatePreviousTask();
    static void dispatchActivateNextTask();
    static void dispatchMoveActiveTaskBackward();
    static void dispatchMoveActiveTaskForward();

    QVariantList systemSettingsActions(QObject *parent) const;

    QActionGroup *m_actionGroup = nullptr;
    KActivities::Consumer *m_activitiesConsumer = nullptr;

    static QVector<Backend *> s_instances;

    int m_screen = -1;
    bool m_hasActiveTask = false;
    bool m_showsOnlyCurrentScreen = false;

    KActivityManagerdPluginsSettings m_activityManagerPluginsSettings;
    KConfigWatcher::Ptr m_activityManagerPluginsSettingsWatcher;
};
