/*
    SPDX-FileCopyrightText: 2021 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QGuiApplication>
#include <QQuickItem>
#include <QTimer>

#include <optional>

/**
 * This class filters child mouse events that move from a current location towards a given edge.
 *
 * The primary use-case being where we have a list of actions that trigger on hover on one side
 * adjacent to a large hit area where a user will want to interact with.
 *
 * Without this filter a user moving their mouse towards the target area will trigger other hover events
 *
 * This item distinguishes mouse moves towards an edge from attempts to select another item in the combo
 * tree.
 *
 * See: https://bjk5.com/post/44698559168/breaking-down-amazons-mega-dropdown
 */
class TriangleMouseFilter : public QQuickItem
{
    Q_OBJECT
    /**
     * The timeout in ms after which the filter is disabled and the current item is selected
     * regardless.
     *
     * The default is 300
     * Setting a negative value disables the timeout
     */
    Q_PROPERTY(int filterTimeOut MEMBER m_filterTimeout NOTIFY filterTimoutChanged)

    /**
     * The edge that we want to filter mouse actions towards.
     * i.e if we have a listview on the left with a submenu on the right, the value
     * will be Qt.RightEdge
     *
     * RTL configurations must be handled explicitly by the caller
     */
    Q_PROPERTY(Qt::Edge edge MEMBER m_edge NOTIFY edgeChanged)

public:
    TriangleMouseFilter(QQuickItem *parent = nullptr);
    ~TriangleMouseFilter() = default;

Q_SIGNALS:
    void filterTimoutChanged();
    void edgeChanged();

protected:
    bool childMouseEventFilter(QQuickItem *item, QEvent *event) override;

private:
    bool filterContains(const QPointF &p) const;
    QTimer m_resetTimer;
    std::optional<QPointF> m_interceptionPos; // point where we started intercepting
    QPointF m_lastCursorPosition;
    QPointer<QQuickItem> m_interceptedHoverItem; // item newly entered but the enter event was intercepted
    std::optional<QPointF> m_interceptedHoverEnterPosition; // position of intercepted enter events
    Qt::Edge m_edge = Qt::RightEdge;
    int m_filterTimeout = 300;
};
