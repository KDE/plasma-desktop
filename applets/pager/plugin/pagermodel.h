/*
    SPDX-FileCopyrightText: 2016 Eike Hein <hein.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PAGERMODEL_H
#define PAGERMODEL_H

#include <config-X11.h>

#if HAVE_X11
#include <QX11Info>
#include <netwm.h>
#endif

#include <QAbstractListModel>
#include <QQmlParserStatus>
#include <qwindowdefs.h>

class QMimeData;

class PagerModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT

    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(PagerType pagerType READ pagerType WRITE setPagerType NOTIFY pagerTypeChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool shouldShowPager READ shouldShowPager NOTIFY shouldShowPagerChanged)
    Q_PROPERTY(bool showDesktop READ showDesktop WRITE setShowDesktop NOTIFY showDesktopChanged)
    Q_PROPERTY(bool showOnlyCurrentScreen READ showOnlyCurrentScreen WRITE setShowOnlyCurrentScreen NOTIFY showOnlyCurrentScreenChanged)
    Q_PROPERTY(QRect screenGeometry READ screenGeometry WRITE setScreenGeometry NOTIFY screenGeometryChanged)
    Q_PROPERTY(int currentPage READ currentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int layoutRows READ layoutRows NOTIFY layoutRowsChanged)
    Q_PROPERTY(QSize pagerItemSize READ pagerItemSize NOTIFY pagerItemSizeChanged)

public:
    enum PagerType {
        VirtualDesktops = 0,
        Activities,
    };
    Q_ENUM(PagerType)

    enum AdditionalRoles {
        TasksModel = Qt::UserRole + 1,
    };
    Q_ENUM(AdditionalRoles)

    explicit PagerModel(QObject *parent = nullptr);
    ~PagerModel() override;

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    PagerType pagerType() const;
    void setPagerType(PagerType type);

    bool enabled() const;
    void setEnabled(bool enabled);

    bool shouldShowPager() const;

    bool showDesktop() const;
    void setShowDesktop(bool show);

    bool showOnlyCurrentScreen() const;
    void setShowOnlyCurrentScreen(bool show);

    QRect screenGeometry() const;
    void setScreenGeometry(const QRect &geometry);

    int currentPage() const;

    int layoutRows() const;
    QSize pagerItemSize() const;

#if HAVE_X11
    QList<WId> stackingOrder() const;
#endif

    Q_INVOKABLE void refresh();

    Q_INVOKABLE void moveWindow(const QModelIndex &window,
                                double x,
                                double y,
                                const QVariant &targetItemId,
                                const QVariant &sourceItemId,
                                qreal widthScaleFactor,
                                qreal heightScaleFactor);
    Q_INVOKABLE void changePage(int page);
    Q_INVOKABLE void drop(QMimeData *mimeData, int modifiers, const QVariant &itemId);
    Q_INVOKABLE void addDesktop();
    Q_INVOKABLE void removeDesktop();

    void classBegin() override;
    void componentComplete() override;

Q_SIGNALS:
    void countChanged() const;
    void pagerTypeChanged() const;
    void enabledChanged() const;
    void shouldShowPagerChanged() const;
    void showDesktopChanged() const;
    void showOnlyCurrentScreenChanged() const;
    void screenGeometryChanged() const;
    void currentPageChanged() const;
    void layoutRowsChanged() const;
    void pagerItemSizeChanged() const;

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif
