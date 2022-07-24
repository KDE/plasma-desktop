/*
    SPDX-FileCopyrightText: 2013-2016 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BACKEND_H
#define BACKEND_H

#include <memory>

#include <QObject>
#include <QRect>

#include <netwm.h>
#include <qwindowdefs.h>

class QAction;
class QActionGroup;
class QQuickItem;
class QQuickWindow;
class QJsonArray;

class AccessibleProgressBar;

namespace KActivities
{
class Consumer;
}

class Backend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem *taskManagerItem READ taskManagerItem WRITE setTaskManagerItem NOTIFY taskManagerItemChanged)
    Q_PROPERTY(bool highlightWindows READ highlightWindows WRITE setHighlightWindows NOTIFY highlightWindowsChanged)
    Q_PROPERTY(bool windowViewAvailable READ windowViewAvailable NOTIFY windowViewAvailableChanged)

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

    bool highlightWindows() const;
    void setHighlightWindows(bool highlight);

    Q_INVOKABLE QVariantList jumpListActions(const QUrl &launcherUrl, QObject *parent);
    Q_INVOKABLE QVariantList placesActions(const QUrl &launcherUrl, bool showAllPlaces, QObject *parent);
    Q_INVOKABLE QVariantList recentDocumentActions(const QUrl &launcherUrl, QObject *parent);
    Q_INVOKABLE void setActionGroup(QAction *action) const;

    Q_INVOKABLE QRect globalRect(QQuickItem *item) const;

    bool windowViewAvailable() const;

    Q_INVOKABLE bool isApplication(const QUrl &url) const;

    Q_INVOKABLE QList<QUrl> jsonArrayToUrlList(const QJsonArray &array) const;

    Q_INVOKABLE void cancelHighlightWindows();

    Q_INVOKABLE qint64 parentPid(qint64 pid) const;

    /**
     * @note workaround for https://bugreports.qt.io/browse/QTBUG-105155
     */
    Q_INVOKABLE void updateProgressBarAccessibleEvent(QObject *progressBarItem);

    static QUrl tryDecodeApplicationsUrl(const QUrl &launcherUrl);
    Q_INVOKABLE static QStringList applicationCategories(const QUrl &launcherUrl);

public Q_SLOTS:
    void activateWindowView(const QVariant &winIds);
    void windowsHovered(const QVariant &winIds, bool hovered);

Q_SIGNALS:
    void taskManagerItemChanged() const;
    void highlightWindowsChanged() const;
    void addLauncher(const QUrl &url) const;
    void windowViewAvailableChanged();

    void showAllPlaces();

private Q_SLOTS:
    void handleRecentDocumentAction() const;

private:
    void updateWindowHighlight();

    QVariantList systemSettingsActions(QObject *parent) const;

    QQuickItem *m_taskManagerItem = nullptr;
    bool m_highlightWindows;
    QStringList m_windowsToHighlight;
    QActionGroup *m_actionGroup = nullptr;
    KActivities::Consumer *m_activitiesConsumer = nullptr;
    bool m_windowViewAvailable = false;

    std::unique_ptr<AccessibleProgressBar> m_progressBarAccessibleIface;
};

#endif
