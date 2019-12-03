/*
 * Copyright © 2003-2007 Fredrik Höglund <fredrik@kde.org>
 * Copyright 2019 Kai Uwe Broulik <kde@broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "previewitem.h"

#include <QHoverEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmapCache>
#include <QQuickWindow>
#include <QStyleFactory>
#include <QWidget>

#include <KColorScheme>
#include <KSharedConfig>

PreviewItem::PreviewItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setAcceptHoverEvents(true);

    // HACK QtCurve deadlocks on application teardown when the Q_GLOBAL_STATIC QFactoryLoader
    // in QStyleFactory is destroyed which destroys all loaded styles prompting QtCurve
    // to disconnect from DBus stalling the application.
    // This also happens before any of the KCM objects are destroyed, so our only chance
    // is cleaning up in response to aboutToQuit
    connect(qApp, &QApplication::aboutToQuit, this, [this] {
        m_style.reset();
    });
}

PreviewItem::~PreviewItem() = default;

void PreviewItem::componentComplete()
{
    QQuickPaintedItem::componentComplete();
    reload();
}

bool PreviewItem::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_widget.data()) {
        switch (event->type()) {
        case QEvent::Show:
        case QEvent::UpdateRequest:
            update();
            break;
        default:
            break;
        }
    }

    return QQuickPaintedItem::eventFilter(watched, event);
}

QString PreviewItem::styleName() const
{
    return m_styleName;
}

void PreviewItem::setStyleName(const QString &styleName)
{
    if (m_styleName == styleName) {
        return;
    }

    m_styleName = styleName;
    reload();
    emit styleNameChanged();
}

bool PreviewItem::isValid() const
{
    return m_style && m_widget;
}

void setStyleRecursively(QWidget *widget, QStyle *style, const QPalette &palette)
{
    // Don't let styles kill the palette for other styles being previewed.
    widget->setPalette(QPalette());
    widget->setPalette(palette);

    widget->setStyle(style);

    const auto children = widget->children();
    for (QObject *child : children) {
        if (child->isWidgetType()) {
            setStyleRecursively(static_cast<QWidget *>(child), style, palette);
        }
    }
}

void PreviewItem::reload()
{
    if (!isComponentComplete()) {
        return;
    }

    const bool oldValid = isValid();

    m_style.reset(QStyleFactory::create(m_styleName));
    if (!m_style) {
        qWarning() << "Failed to load style" << m_styleName;
        if (oldValid != isValid()) {
            emit validChanged();
        }
        return;
    }

    m_widget.reset(new QWidget);
    // Don't actually show the widget as a separate window when calling show()
    m_widget->setAttribute(Qt::WA_DontShowOnScreen);
    // Do not wait for this widget to close before the app closes
    m_widget->setAttribute(Qt::WA_QuitOnClose, false);

    m_ui.setupUi(m_widget.data());

    // Prevent Qt from wrongly caching radio button images
    QPixmapCache::clear();

    QPalette palette(KColorScheme::createApplicationPalette(KSharedConfig::openConfig()));
    m_style->polish(palette);

    // HACK Needed so the previews look like their window is active
    // The previews don't have a parent (we're in QML, after all, there is no QWidget* to parent it to)
    // so QWidget::isActiveWindow() always returns false making the widget look dull
    // You still won't get hover effects in some themes (those that don't do that for inactive windows)
    // but at least at a glance it looks fine...
    for (int i = 0; i < QPalette::NColorRoles; ++i) {
        const auto role = static_cast<QPalette::ColorRole>(i);
        palette.setColor(QPalette::Inactive, role, palette.color(QPalette::Active, role));
    }

    setStyleRecursively(m_widget.data(), m_style.data(), palette);

    m_widget->ensurePolished();

    m_widget->resize(qRound(width()), qRound(height()));

    m_widget->installEventFilter(this);

    m_widget->show();

    const auto sizeHint = m_widget->sizeHint();
    setImplicitSize(sizeHint.width(), sizeHint.height());

    if (oldValid != isValid()) {
        emit validChanged();
    }
}

void PreviewItem::paint(QPainter *painter)
{
    if (m_widget && m_widget->isVisible()) {
        m_widget->render(painter);
    }
}

void PreviewItem::hoverMoveEvent(QHoverEvent *event)
{
    sendHoverEvent(event);
}

void PreviewItem::hoverLeaveEvent(QHoverEvent *event)
{
    if (m_lastWidgetUnderMouse) {
        dispatchEnterLeave(nullptr, m_lastWidgetUnderMouse, mapToGlobal(event->pos()));
        m_lastWidgetUnderMouse = nullptr;
    }
}

void PreviewItem::sendHoverEvent(QHoverEvent *event)
{
    if (!m_widget || !m_widget->isVisible()) {
        return;
    }

    QPointF pos = event->pos();

    QWidget *child = m_widget->childAt(pos.toPoint());
    QWidget *receiver = child ? child : m_widget.data();

    dispatchEnterLeave(receiver, m_lastWidgetUnderMouse, mapToGlobal(event->pos()));

    m_lastWidgetUnderMouse = receiver;

    pos = receiver->mapFrom(m_widget.data(), pos.toPoint());

    QMouseEvent mouseEvent(QEvent::MouseMove, pos, receiver->mapTo(receiver->topLevelWidget(), pos.toPoint()),
                           receiver->mapToGlobal(pos.toPoint()),
                           Qt::NoButton, {} /*buttons*/, event->modifiers());

    qApp->sendEvent(receiver, &mouseEvent);

    event->setAccepted(mouseEvent.isAccepted());
}

// Simplified copy of QApplicationPrivate::dispatchEnterLeave
void PreviewItem::dispatchEnterLeave(QWidget *enter, QWidget *leave, const QPointF &globalPosF)
{
    if ((!enter && !leave) || (enter == leave)) {
        return;
    }

    QWidgetList leaveList;
    QWidgetList enterList;

    bool sameWindow = leave && enter && leave->window() == enter->window();
    if (leave && !sameWindow) {
        auto *w = leave;
        do {
            leaveList.append(w);
        } while (!w->isWindow() && (w = w->parentWidget()));
    }
    if (enter && !sameWindow) {
        auto *w = enter;
        do {
            enterList.append(w);
        } while (!w->isWindow() && (w = w->parentWidget()));
    }
    if (sameWindow) {
        int enterDepth = 0;
        int leaveDepth = 0;
        auto *e = enter;
        while (!e->isWindow() && (e = e->parentWidget()))
            enterDepth++;
        auto *l = leave;
        while (!l->isWindow() && (l = l->parentWidget()))
            leaveDepth++;
        QWidget* wenter = enter;
        QWidget* wleave = leave;
        while (enterDepth > leaveDepth) {
            wenter = wenter->parentWidget();
            enterDepth--;
        }
        while (leaveDepth > enterDepth) {
            wleave = wleave->parentWidget();
            leaveDepth--;
        }
        while (!wenter->isWindow() && wenter != wleave) {
            wenter = wenter->parentWidget();
            wleave = wleave->parentWidget();
        }

        for (auto *w = leave; w != wleave; w = w->parentWidget())
            leaveList.append(w);

        for (auto *w = enter; w != wenter; w = w->parentWidget())
            enterList.append(w);
    }

    const QPoint globalPos = globalPosF.toPoint();

    QEvent leaveEvent(QEvent::Leave);
    for (int i = 0; i < leaveList.size(); ++i) {
        auto *w = leaveList.at(i);
        QApplication::sendEvent(w, &leaveEvent);
        if (w->testAttribute(Qt::WA_Hover)) {
            QHoverEvent he(QEvent::HoverLeave, QPoint(-1, -1), w->mapFromGlobal(globalPos),
                            QApplication::keyboardModifiers());
            QApplication::sendEvent(w, &he);
        }
    }
    if (!enterList.isEmpty()) {
        const QPoint windowPos = qAsConst(enterList).back()->window()->mapFromGlobal(globalPos);
        for (auto it = enterList.crbegin(), end = enterList.crend(); it != end; ++it) {
            auto *w = *it;
            const QPointF localPos = w->mapFromGlobal(globalPos);
            QEnterEvent enterEvent(localPos, windowPos, globalPosF);
            QApplication::sendEvent(w, &enterEvent);
            if (w->testAttribute(Qt::WA_Hover)) {
                QHoverEvent he(QEvent::HoverEnter, localPos, QPoint(-1, -1),
                               QApplication::keyboardModifiers());
                QApplication::sendEvent(w, &he);
            }
        }
    }
}

void PreviewItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (m_widget && newGeometry != oldGeometry) {
        m_widget->resize(qRound(newGeometry.width()), qRound(newGeometry.height()));
    }

    QQuickPaintedItem::geometryChanged(newGeometry, oldGeometry);
}

