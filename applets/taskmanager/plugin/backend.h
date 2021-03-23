/***************************************************************************
 *   Copyright (C) 2013-2016 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QRect>

#include <netwm.h>
#include <qwindowdefs.h>

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

    Q_PROPERTY(QQuickItem *taskManagerItem READ taskManagerItem WRITE setTaskManagerItem NOTIFY taskManagerItemChanged)
    Q_PROPERTY(QQuickWindow *groupDialog READ groupDialog WRITE setGroupDialog NOTIFY groupDialogChanged)
    Q_PROPERTY(bool highlightWindows READ highlightWindows WRITE setHighlightWindows NOTIFY highlightWindowsChanged)
    Q_PROPERTY(bool canPresentWindows READ canPresentWindows NOTIFY canPresentWindowsChanged)

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

    explicit Backend(QObject *parent = nullptr);
    ~Backend() override;

    QQuickItem *taskManagerItem() const;
    void setTaskManagerItem(QQuickItem *item);

    QQuickWindow *groupDialog() const;
    void setGroupDialog(QQuickWindow *dialog);

    bool highlightWindows() const;
    void setHighlightWindows(bool highlight);

    Q_INVOKABLE QVariantList jumpListActions(const QUrl &launcherUrl, QObject *parent);
    Q_INVOKABLE QVariantList placesActions(const QUrl &launcherUrl, bool showAllPlaces, QObject *parent);
    Q_INVOKABLE QVariantList recentDocumentActions(const QUrl &launcherUrl, QObject *parent);
    Q_INVOKABLE void setActionGroup(QAction *action) const;

    Q_INVOKABLE QRect globalRect(QQuickItem *item) const;

    Q_INVOKABLE void ungrabMouse(QQuickItem *item) const;

    bool canPresentWindows() const;

    Q_INVOKABLE bool isApplication(const QUrl &url) const;

    Q_INVOKABLE QList<QUrl> jsonArrayToUrlList(const QJsonArray &array) const;

    Q_INVOKABLE void cancelHighlightWindows();

    Q_INVOKABLE qint64 parentPid(qint64 pid) const;

    static QUrl tryDecodeApplicationsUrl(const QUrl &launcherUrl);

public Q_SLOTS:
    void presentWindows(const QVariant &winIds);
    void windowsHovered(const QVariant &winIds, bool hovered);

Q_SIGNALS:
    void taskManagerItemChanged() const;
    void groupDialogChanged() const;
    void highlightWindowsChanged() const;
    void addLauncher(const QUrl &url) const;
    void canPresentWindowsChanged();

    void showAllPlaces();

private Q_SLOTS:
    void handleRecentDocumentAction() const;

private:
    void updateWindowHighlight();

    QVariantList systemSettingsActions(QObject *parent) const;

    QQuickItem *m_taskManagerItem = nullptr;
    QQuickWindow *m_groupDialog = nullptr;
    bool m_highlightWindows;
    QStringList m_windowsToHighlight;
    QActionGroup *m_actionGroup = nullptr;
    KActivities::Consumer *m_activitiesConsumer = nullptr;
    bool m_canPresentWindows = false;
};

#endif
