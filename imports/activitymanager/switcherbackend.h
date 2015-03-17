/*
 *   Copyright (C) 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SWITCHER_BACKEND_H
#define SWITCHER_BACKEND_H

// Qt
#include <QObject>
#include <QHash>
#include <QKeySequence>
#include <QTimer>
#include <QPixmap>
#include <QJSValue>

// KDE
#include <kactivities/controller.h>
#include <kimagecache.h>

class QAction;
class QQmlEngine;
class QJSEngine;

namespace KIO { class PreviewJob; }

class SwitcherBackend : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool shouldShowSwitcher READ shouldShowSwitcher WRITE setShouldShowSwitcher NOTIFY shouldShowSwitcherChanged)


public:
    SwitcherBackend(QObject *parent = Q_NULLPTR);
    ~SwitcherBackend();

    static QObject *instance(QQmlEngine *engine, QJSEngine *scriptEngine);

Q_SIGNALS:
    void showSwitchNotification(const QString &id, const QString &name, const QString &icon);
    void shouldShowSwitcherChanged(bool value);

public Q_SLOTS:
    void init();

    bool shouldShowSwitcher() const;
    void setShouldShowSwitcher(const bool &shouldShowSwitcher);

    QPixmap wallpaperThumbnail(const QString &path, int width, int height,
            const QJSValue &callback);

private:
    template <typename Handler>
    inline void registerShortcut(const QString &actionName, const QString &name,
                                 const QKeySequence &shortcut,
                                 Handler &&handler);


    enum Direction {
        Next,
        Previous
    };
    void switchToActivity(Direction i);

private Q_SLOTS:
    void keybdSwitchToNextActivity();
    void keybdSwitchToPreviousActivity();
    void keybdSwitchedToAnotherActivity();

    void showActivitySwitcherIfNeeded();

    void currentActivityChangedSlot(const QString &id);

private:
    QHash<QString, QKeySequence> m_actionShortcut;
    QAction *m_lastInvokedAction;
    KActivities::Controller m_activities;
    bool m_shouldShowSwitcher;
    QTimer m_modKeyPollingTimer;

    KImageCache *m_wallpaperCache;
    QSet<QUrl> m_previewJobs;

};

#endif // SWITCHER_BACKEND_H
