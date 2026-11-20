/*
    SPDX-FileCopyrightText: 2013-2016 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KConfigWatcher>

#include <QAction>
#include <QObject>
#include <QQuickItem>
#include <QRect>

#include <memory>
#include <netwm.h>
#include <qqmlregistration.h>
#include <qwindowdefs.h>

#include "kactivitymanagerd_plugins_settings.h"

class KActionCollection;
class QActionGroup;
class QQuickItem;
class QQuickWindow;
class QJsonArray;
class QTimer;
class KModifierKeyInfo;

namespace KActivities
{
class Consumer;
}

class MetaTracker
{
public:
    explicit MetaTracker();

private:
    std::unique_ptr<KModifierKeyInfo> m_modifierInfo;
    std::unique_ptr<QTimer> m_timer;
};

class Backend : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    friend MetaTracker;

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
    Q_PROPERTY(bool showShortcutHelper MEMBER m_showShortcutHelper NOTIFY showShortcutHelperChanged)
    Q_PROPERTY(QList<QChar> activationKeys READ activationKeys NOTIFY activationKeysChanged)

    explicit Backend(QObject *parent = nullptr);
    ~Backend() override;

    Q_INVOKABLE QVariantList jumpListActions(const QUrl &launcherUrl, QObject *parent);
    Q_INVOKABLE QVariantList placesActions(const QUrl &launcherUrl, bool showAllPlaces, QObject *parent);
    Q_INVOKABLE QVariantList recentDocumentActions(const QUrl &launcherUrl, QObject *parent);
    Q_INVOKABLE void setActionGroup(QAction *action) const;

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

    QList<QChar> activationKeys() const;

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
    void showShortcutHelperChanged();
    void activationKeysChanged();

private Q_SLOTS:
    void handleRecentDocumentAction() const;

private:
    static void setupShortcuts();
    static Backend *findTargetBackend();
    static Backend *findIndexBackend();
    static void dispatchMetaHold(bool held);
    static void dispatchActivateTaskAtIndex(int index);
    static void dispatchActivatePreviousTask();
    static void dispatchActivateNextTask();
    static void dispatchMoveActiveTaskBackward();
    static void dispatchMoveActiveTaskForward();
    static bool checkHighlightableShortcuts();
    static void setActivationKeys(QList<QChar> newActivationKeys);

    QVariantList systemSettingsActions(QObject *parent) const;
    void setShowShortcutHelper(bool show);

    QActionGroup *m_actionGroup = nullptr;
    KActivities::Consumer *m_activitiesConsumer = nullptr;

    static QVector<Backend *> s_instances;

    static MetaTracker s_metaTracker;
    static QList<QChar> s_activationKeys;
    int m_screen = -1;
    bool m_hasActiveTask = false;
    bool m_showsOnlyCurrentScreen = false;
    bool m_showShortcutHelper = false;

    KActivityManagerdPluginsSettings m_activityManagerPluginsSettings;
    KConfigWatcher::Ptr m_activityManagerPluginsSettingsWatcher;
};
