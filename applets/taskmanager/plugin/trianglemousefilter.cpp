/*
    SPDX-FileCopyrightText: 2021 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "trianglemousefilter.h"

#include <QPolygonF>

TriangleMouseFilter::TriangleMouseFilter(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFiltersChildMouseEvents(true);

    m_resetTimer.setSingleShot(true);
    connect(&m_resetTimer, &QTimer::timeout, this, [this]() {
        m_interceptionPos.reset();
        if (!m_interceptedHoverItem) {
            return;
        }
        if (m_interceptedHoverEnterPosition) {
            const auto targetPosition = mapToItem(m_interceptedHoverItem, m_interceptedHoverEnterPosition.value());
            QHoverEvent e(QEvent::HoverEnter, targetPosition, targetPosition);
            qApp->sendEvent(m_interceptedHoverItem, &e);
            m_interceptedHoverEnterPosition.reset();
        }
        const auto targetPosition = mapToItem(m_interceptedHoverItem, m_lastCursorPosition);
        QHoverEvent e(QEvent::HoverMove, targetPosition, targetPosition);
        qApp->sendEvent(m_interceptedHoverItem, &e);
    });
};

bool TriangleMouseFilter::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    switch (event->type()) {
    case QEvent::HoverLeave:
        if (!m_interceptedHoverItem) {
            return false;
        }
        if (item == m_interceptedHoverItem.data()) {
            m_interceptedHoverItem.clear();
            return false;
        }
        return true;
    case QEvent::HoverEnter:
    case QEvent::HoverMove: {
        QHoverEvent *he = static_cast<QHoverEvent *>(event);

        const QPointF position = item->mapToItem(this, he->posF());

        if (filterContains(position)) {
            if (event->type() == QEvent::HoverEnter) {
                m_interceptedHoverEnterPosition = position;
                m_interceptedHoverItem = item;
            }

            if (m_filterTimeout > 0) {
                m_resetTimer.start(m_filterTimeout);
            }

            m_lastCursorPosition = position;
            event->setAccepted(true);

            return true;
        } else {
            // this clause means that we block focus when first entering a given position
            // in the case of kickoff it's so that we can move the mouse from the bottom tabbar to the side view
            // if using this in a more general setting we might want to make this guarded by an option
            if (event->type() == QEvent::HoverEnter && !m_interceptionPos) {
                m_interceptedHoverItem = item;
                m_interceptedHoverEnterPosition = position;
                if (m_filterTimeout > 0) {
                    m_resetTimer.start(m_filterTimeout);
                }
                event->setAccepted(true);
                m_lastCursorPosition = position;
                m_interceptionPos = position;
                return true;
            }

            m_interceptionPos = position;
            m_lastCursorPosition = position;

            // if we are no longer inhibiting events and have previously intercepted a hover enter
            // we manually send the hover enter to that item
            if (event->type() == QEvent::HoverMove && m_interceptedHoverItem) {
                const auto targetPosition = mapToItem(m_interceptedHoverItem, position);
                QHoverEvent e(QEvent::HoverEnter, targetPosition, targetPosition);
                qApp->sendEvent(m_interceptedHoverItem, &e);
                m_interceptedHoverItem.clear();
            }

            event->setAccepted(false);
            return false;
        }
    }
    default:
        return false;
    }
}

bool TriangleMouseFilter::filterContains(const QPointF &p) const
{
    if (!m_interceptionPos) {
        return false;
    }

    // We add some jitter protection by extending our triangle out slight past the mouse position in the opposite direction of the edge;
    const int jitterThreshold = 3;

    // QPolygonF.contains returns false if we're on the edge, so we pad our main item
    const QRectF shape = QRect(-1, -1, width() + 1, height() + 1);

    QPolygonF poly;

    switch (m_edge) {
    case Qt::RightEdge:
        poly << m_interceptionPos.value() + QPointF(-jitterThreshold, 0) << shape.topRight() << shape.bottomRight();
        break;
    case Qt::TopEdge:
        poly << m_interceptionPos.value() + QPointF(0, -jitterThreshold) << shape.topLeft() << shape.topRight();
        break;
    case Qt::LeftEdge:
        poly << m_interceptionPos.value() + QPointF(jitterThreshold, 0) << shape.topLeft() << shape.bottomLeft();
        break;
    case Qt::BottomEdge:
        poly << m_interceptionPos.value() + QPointF(0, jitterThreshold) << shape.bottomLeft() << shape.bottomRight();
    }

    return poly.containsPoint(p, Qt::OddEvenFill);
}
