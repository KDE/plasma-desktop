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

/**
 * This class determines if the activity switcher SHOULD
 * be shown. For example, if activity is cycled through
 * with the keyboard shortcut, or drop mode is activated,
 * this class will let anyone know if the switcher would
 * be helpful to have on the screen.
 *
 * This class doesn't directly control the visibility of
 * the switcher.
 */
class SwitcherBackend : public QObject
{
    Q_OBJECT

    /**
     * If the activity switcher should be shown.
     * Updated by SwitcherBackend based on keyboard activity switching.
     */
    Q_PROPERTY(bool shouldShowSwitcher READ shouldShowSwitcher NOTIFY shouldShowSwitcherChanged)
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

    /**
     * Let the backend know that the activity switcher
     * is no longer visible. Whatever reason shouldShowSwitcher
     * is true for should be invalidated.
     */
    void switcherNoLongerVisible();

    QAbstractItemModel *runningActivitiesModel() const;
    QAbstractItemModel *stoppedActivitiesModel() const;

    void setCurrentActivity(const QString &activity);
    void stopActivity(const QString &activity);

    void setDropMode(bool value);
    void drop(QMimeData *mimeData, int modifiers, const QVariant &activityId);
    void dropCopy(QMimeData *mimeData, const QVariant &activityId);
    void dropMove(QMimeData *mimeData, const QVariant &activityId);
    bool dropEnabled() const;

    /**
     * Note: The activity manager and activity switcher
     * are the same component. The manager just adds a header and footer
     * and the inactive activities model view.
     */
    void toggleActivityManager() const;

private:
    template<typename Handler>
    inline void registerShortcut(const QString &actionName, const QString &name, const QKeySequence &shortcut, Handler &&handler);

    enum Direction {
        Next,
        Previous,
    };

    /**
     * Set the current system activity based on
     * the relative position to the current activity
     * on the switcher panel.
     */
    void switchToActivity(Direction i);

private Q_SLOTS:
    /**
     * Switches through activities.
     * Order based on most recently used.
     */
    void keybdWalkActivityForward();
    void keybdWalkActivityBackward();

    /**
     * Log the new activity we switched to and the time.
     */
    void onCurrentActivityChanged(const QString &id);

private:
    QAction *m_lastInvokedAction = nullptr;
    /**
     * Used for polling the mod key so we can keep
     * the switcher visible until it's released.
     *
     * nullptr unless state == KeyboardSwitch
     */
    std::unique_ptr<QRasterWindow> m_inputWindow;
    /**
     * On wayland, we only get modifier info
     * if we have the focused window. Call this after
     * switching activity to make sure we're still
     * getting modifier info. Does nothing on xorg.
     */
    void makeSureWeGetModifiers();
    bool areModifiersPressed(const QKeySequence &seq);
    KActivities::Controller m_activities;
    QTimer m_modKeyPollingTimer;
    QString m_previousActivity;

    /**
     * The internal reason that the switcher should be shown.
     */
    enum class State {
        /*
         * The switcher doesn't need to be seen.
         */
        NotNeeded,
        /*
         * The switcher is visible because user
         * switched activity with keyboard shortcut.
         */
        KeyboardSwitch,
        /*
         * Visible because dropmode was activated.
         * This option replaces the KeyboardSwitch
         * state when activated. When deactivated,
         * we skip past KeyboardSwitch to notneeded.
         */
        DropModeActive,
    } m_shouldShowSwitcher;

    /**
     * Update the internal reason that we should show the
     * switcher while enforcing the state
     * rules and updating the shouldShowSwitcher property.
     *
     * @param state The reason we're saying is or isn't
     *          currently relevant.
     *          Can't be NotNeeded.
     * @param active If the reason is relevant or not.
     */
    void trySetState(State state, bool active);

    QTimer m_switcherHider;
    QTimer m_dropModeHider;

    SortedActivitiesModel *m_runningActivitiesModel = nullptr;
    SortedActivitiesModel *m_stoppedActivitiesModel = nullptr;
};

#endif // SWITCHER_BACKEND_H
