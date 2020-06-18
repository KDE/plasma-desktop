/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "previewitem.h"
#include "previewbridge.h"
#include "previewsettings.h"
#include "previewclient.h"
#include <KDecoration2/Decoration>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationShadow>
#include <KDecoration2/DecoratedClient>
#include <QCoreApplication>
#include <QCursor>
#include <QPainter>
#include <QQmlContext>
#include <QQmlEngine>

#include <cmath>

#include <QDebug>

namespace KDecoration2
{
namespace Preview
{

PreviewItem::PreviewItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_decoration(nullptr)
    , m_windowColor(QPalette().window().color())
{
    setAcceptHoverEvents(true);
    setFiltersChildMouseEvents(true);
    setAcceptedMouseButtons(Qt::MouseButtons(~Qt::NoButton));
    connect(this, &PreviewItem::widthChanged, this, &PreviewItem::syncSize);
    connect(this, &PreviewItem::heightChanged, this, &PreviewItem::syncSize);
    connect(this, &PreviewItem::bridgeChanged, this, &PreviewItem::createDecoration);
    connect(this, &PreviewItem::settingsChanged, this, &PreviewItem::createDecoration);
}

PreviewItem::~PreviewItem()
{
    m_decoration->deleteLater();
    if (m_bridge){
        m_bridge->unregisterPreviewItem(this);
    }
}

void PreviewItem::componentComplete()
{
    QQuickPaintedItem::componentComplete();
    createDecoration();
    if (m_decoration) {
        m_decoration->setSettings(m_settings->settings());
        m_decoration->init();
        syncSize();
    }
}

void PreviewItem::createDecoration()
{
    if (m_bridge.isNull() || m_settings.isNull() || m_decoration) {
        return;
    }
    Decoration *decoration = m_bridge->createDecoration(nullptr);
    m_client = m_bridge->lastCreatedClient();
    setDecoration(decoration);
}

Decoration *PreviewItem::decoration() const
{
    return m_decoration;
}

void PreviewItem::setDecoration(Decoration *deco)
{
    if (!deco || m_decoration == deco) {
        return;
    }

    m_decoration = deco;
    m_decoration->setProperty("visualParent", QVariant::fromValue(this));
    connect(m_decoration, &Decoration::bordersChanged, this, &PreviewItem::syncSize);
    connect(m_decoration, &Decoration::shadowChanged, this, &PreviewItem::syncSize);
    connect(m_decoration, &Decoration::shadowChanged, this, &PreviewItem::shadowChanged);
    emit decorationChanged(m_decoration);
}

QColor PreviewItem::windowColor() const
{
    return m_windowColor;
}

void PreviewItem::setWindowColor(const QColor &color)
{
    if (m_windowColor == color) {
        return;
    }
    m_windowColor = color;
    emit windowColorChanged(m_windowColor);
    update();
}

void PreviewItem::paint(QPainter *painter)
{
    if (!m_decoration) {
        return;
    }
    int paddingLeft   = 0;
    int paddingTop    = 0;
    int paddingRight  = 0;
    int paddingBottom = 0;
    paintShadow(painter, paddingLeft, paddingRight, paddingTop, paddingBottom);
    m_decoration->paint(painter, QRect(0, 0, width(), height()));
    if (m_drawBackground) {
        painter->fillRect(m_decoration->borderLeft(), m_decoration->borderTop(),
                        width() - m_decoration->borderLeft() - m_decoration->borderRight() - paddingLeft - paddingRight,
                        height() - m_decoration->borderTop() - m_decoration->borderBottom() - paddingTop - paddingBottom,
                        m_windowColor);
    }
}

void PreviewItem::paintShadow(QPainter *painter, int &paddingLeft, int &paddingRight, int &paddingTop, int &paddingBottom)
{
    const auto &shadow = ((const Decoration*)(m_decoration))->shadow();
    if (!shadow) {
        return;
    }

    paddingLeft   = shadow->paddingLeft();
    paddingTop    = shadow->paddingTop();
    paddingRight  = shadow->paddingRight();
    paddingBottom = shadow->paddingBottom();

    const QImage shadowPixmap = shadow->shadow();
    if (shadowPixmap.isNull()) {
        return;
    }

    const QRect outerRect(-paddingLeft, -paddingTop, width(), height());
    const QRect shadowRect(shadowPixmap.rect());

    const QSize topLeftSize(shadow->topLeftGeometry().size());
    QRect topLeftTarget(
        QPoint(outerRect.x(), outerRect.y()),
        topLeftSize);

    const QSize topRightSize(shadow->topRightGeometry().size());
    QRect topRightTarget(
        QPoint(outerRect.x() + outerRect.width() - topRightSize.width(),
               outerRect.y()),
        topRightSize);

    const QSize bottomRightSize(shadow->bottomRightGeometry().size());
    QRect bottomRightTarget(
        QPoint(outerRect.x() + outerRect.width() - bottomRightSize.width(),
               outerRect.y() + outerRect.height() - bottomRightSize.height()),
        bottomRightSize);

    const QSize bottomLeftSize(shadow->bottomLeftGeometry().size());
    QRect bottomLeftTarget(
        QPoint(outerRect.x(),
               outerRect.y() + outerRect.height() - bottomLeftSize.height()),
        bottomLeftSize);

    // Re-distribute the corner tiles so no one of them is overlapping with others.
    // By doing this, we assume that shadow's corner tiles are symmetric
    // and it is OK to not draw top/right/bottom/left tile between corners.
    // For example, let's say top-left and top-right tiles are overlapping.
    // In that case, the right side of the top-left tile will be shifted to left,
    // the left side of the top-right tile will shifted to right, and the top
    // tile won't be rendered.
    bool drawTop = true;
    if (topLeftTarget.x() + topLeftTarget.width() >= topRightTarget.x()) {
        const float halfOverlap = qAbs(topLeftTarget.x() + topLeftTarget.width() - topRightTarget.x()) / 2.0f;
        topLeftTarget.setRight(topLeftTarget.right() - std::floor(halfOverlap));
        topRightTarget.setLeft(topRightTarget.left() + std::ceil(halfOverlap));
        drawTop = false;
    }

    bool drawRight = true;
    if (topRightTarget.y() + topRightTarget.height() >= bottomRightTarget.y()) {
        const float halfOverlap = qAbs(topRightTarget.y() + topRightTarget.height() - bottomRightTarget.y()) / 2.0f;
        topRightTarget.setBottom(topRightTarget.bottom() - std::floor(halfOverlap));
        bottomRightTarget.setTop(bottomRightTarget.top() + std::ceil(halfOverlap));
        drawRight = false;
    }

    bool drawBottom = true;
    if (bottomLeftTarget.x() + bottomLeftTarget.width() >= bottomRightTarget.x()) {
        const float halfOverlap = qAbs(bottomLeftTarget.x() + bottomLeftTarget.width() - bottomRightTarget.x()) / 2.0f;
        bottomLeftTarget.setRight(bottomLeftTarget.right() - std::floor(halfOverlap));
        bottomRightTarget.setLeft(bottomRightTarget.left() + std::ceil(halfOverlap));
        drawBottom = false;
    }

    bool drawLeft = true;
    if (topLeftTarget.y() + topLeftTarget.height() >= bottomLeftTarget.y()) {
        const float halfOverlap = qAbs(topLeftTarget.y() + topLeftTarget.height() - bottomLeftTarget.y()) / 2.0f;
        topLeftTarget.setBottom(topLeftTarget.bottom() - std::floor(halfOverlap));
        bottomLeftTarget.setTop(bottomLeftTarget.top() + std::ceil(halfOverlap));
        drawLeft = false;
    }

    painter->translate(paddingLeft, paddingTop);

    painter->drawImage(topLeftTarget, shadowPixmap,
                       QRect(QPoint(0, 0), topLeftTarget.size()));

    painter->drawImage(topRightTarget, shadowPixmap,
                      QRect(QPoint(shadowRect.width() - topRightTarget.width(), 0),
                            topRightTarget.size()));

    painter->drawImage(bottomRightTarget, shadowPixmap,
                       QRect(QPoint(shadowRect.width() - bottomRightTarget.width(),
                                    shadowRect.height() - bottomRightTarget.height()),
                             bottomRightTarget.size()));

    painter->drawImage(bottomLeftTarget, shadowPixmap,
                       QRect(QPoint(0, shadowRect.height() - bottomLeftTarget.height()),
                             bottomLeftTarget.size()));

    if (drawTop) {
        QRect topTarget(topLeftTarget.x() + topLeftTarget.width(),
                        topLeftTarget.y(),
                        topRightTarget.x() - topLeftTarget.x() - topLeftTarget.width(),
                        topRightTarget.height());
        QRect topSource(shadow->topGeometry());
        topSource.setHeight(topTarget.height());
        topSource.moveTop(shadowRect.top());
        painter->drawImage(topTarget, shadowPixmap, topSource);
    }

    if (drawRight) {
        QRect rightTarget(topRightTarget.x(),
                          topRightTarget.y() + topRightTarget.height(),
                          topRightTarget.width(),
                          bottomRightTarget.y() - topRightTarget.y() - topRightTarget.height());
        QRect rightSource(shadow->rightGeometry());
        rightSource.setWidth(rightTarget.width());
        rightSource.moveRight(shadowRect.right());
        painter->drawImage(rightTarget, shadowPixmap, rightSource);
    }

    if (drawBottom) {
        QRect bottomTarget(bottomLeftTarget.x() + bottomLeftTarget.width(),
                           bottomLeftTarget.y(),
                           bottomRightTarget.x() - bottomLeftTarget.x() - bottomLeftTarget.width(),
                           bottomRightTarget.height());
        QRect bottomSource(shadow->bottomGeometry());
        bottomSource.setHeight(bottomTarget.height());
        bottomSource.moveBottom(shadowRect.bottom());
        painter->drawImage(bottomTarget, shadowPixmap, bottomSource);
    }

    if (drawLeft) {
       QRect leftTarget(topLeftTarget.x(),
                        topLeftTarget.y() + topLeftTarget.height(),
                        topLeftTarget.width(),
                        bottomLeftTarget.y() - topLeftTarget.y() - topLeftTarget.height());
        QRect leftSource(shadow->leftGeometry());
        leftSource.setWidth(leftTarget.width());
        leftSource.moveLeft(shadowRect.left());
        painter->drawImage(leftTarget, shadowPixmap, leftSource);
    }
}

void PreviewItem::mouseDoubleClickEvent(QMouseEvent *event)
{
    const auto &shadow = m_decoration->shadow();
    if (shadow) {
        QMouseEvent e(event->type(),
                      event->localPos() - QPointF(shadow->paddingLeft(), shadow->paddingTop()),
                      event->button(),
                      event->buttons(),
                      event->modifiers());
        QCoreApplication::instance()->sendEvent(decoration(), &e);
    } else {
        QCoreApplication::instance()->sendEvent(decoration(), event);
    }
}

void PreviewItem::mousePressEvent(QMouseEvent *event)
{
    const auto &shadow = m_decoration->shadow();
    if (shadow) {
        QMouseEvent e(event->type(),
                      event->localPos() - QPointF(shadow->paddingLeft(), shadow->paddingTop()),
                      event->button(),
                      event->buttons(),
                      event->modifiers());
        QCoreApplication::instance()->sendEvent(decoration(), &e);
    } else {
        QCoreApplication::instance()->sendEvent(decoration(), event);
    }
}

void PreviewItem::mouseReleaseEvent(QMouseEvent *event)
{
    const auto &shadow = m_decoration->shadow();
    if (shadow) {
        QMouseEvent e(event->type(),
                      event->localPos() - QPointF(shadow->paddingLeft(), shadow->paddingTop()),
                      event->button(),
                      event->buttons(),
                      event->modifiers());
        QCoreApplication::instance()->sendEvent(decoration(), &e);
    } else {
        QCoreApplication::instance()->sendEvent(decoration(), event);
    }
}

void PreviewItem::mouseMoveEvent(QMouseEvent *event)
{
    const auto &shadow = m_decoration->shadow();
    if (shadow) {
        QMouseEvent e(event->type(),
                      event->localPos() - QPointF(shadow->paddingLeft(), shadow->paddingTop()),
                      event->button(),
                      event->buttons(),
                      event->modifiers());
        QCoreApplication::instance()->sendEvent(decoration(), &e);
    } else {
        QCoreApplication::instance()->sendEvent(decoration(), event);
    }
}

void PreviewItem::hoverEnterEvent(QHoverEvent *event)
{
    const auto &shadow = m_decoration->shadow();
    if (shadow) {
        QHoverEvent e(event->type(),
                      event->posF() - QPointF(shadow->paddingLeft(), shadow->paddingTop()),
                      event->oldPosF() - QPointF(shadow->paddingLeft(), shadow->paddingTop()),
                      event->modifiers());
        QCoreApplication::instance()->sendEvent(decoration(), &e);
    } else {
        QCoreApplication::instance()->sendEvent(decoration(), event);
    }
}

void PreviewItem::hoverLeaveEvent(QHoverEvent *event)
{
    const auto &shadow = m_decoration->shadow();
    if (shadow) {
        QHoverEvent e(event->type(),
                      event->posF() - QPointF(shadow->paddingLeft(), shadow->paddingTop()),
                      event->oldPosF() - QPointF(shadow->paddingLeft(), shadow->paddingTop()),
                      event->modifiers());
        QCoreApplication::instance()->sendEvent(decoration(), &e);
    } else {
        QCoreApplication::instance()->sendEvent(decoration(), event);
    }
}

void PreviewItem::hoverMoveEvent(QHoverEvent *event)
{
    const auto &shadow = m_decoration->shadow();
    if (shadow) {
        QHoverEvent e(event->type(),
                      event->posF() - QPointF(shadow->paddingLeft(), shadow->paddingTop()),
                      event->oldPosF() - QPointF(shadow->paddingLeft(), shadow->paddingTop()),
                      event->modifiers());
        QCoreApplication::instance()->sendEvent(decoration(), &e);
    } else {
        QCoreApplication::instance()->sendEvent(decoration(), event);
    }
}

bool PreviewItem::isDrawingBackground() const
{
    return m_drawBackground;
}

void PreviewItem::setDrawingBackground(bool set)
{
    if (m_drawBackground == set) {
        return;
    }
    m_drawBackground = set;
    emit drawingBackgroundChanged(set);
}

PreviewBridge *PreviewItem::bridge() const
{
    return m_bridge.data();
}

void PreviewItem::setBridge(PreviewBridge *bridge)
{
    if (m_bridge == bridge) {
        return;
    }
    if (m_bridge) {
        m_bridge->unregisterPreviewItem(this);
    }
    m_bridge = bridge;
    if (m_bridge) {
        m_bridge->registerPreviewItem(this);
    }
    emit bridgeChanged();
}

Settings *PreviewItem::settings() const
{
    return m_settings.data();
}

void PreviewItem::setSettings(Settings *settings)
{
    if (m_settings == settings) {
        return;
    }
    m_settings = settings;
    emit settingsChanged();
}

PreviewClient *PreviewItem::client()
{
    return m_client.data();
}

void PreviewItem::syncSize()
{
    if (!m_client) {
        return;
    }
    int widthOffset = 0;
    int heightOffset = 0;
    auto shadow = m_decoration->shadow();
    if (shadow) {
        widthOffset = shadow->paddingLeft() + shadow->paddingRight();
        heightOffset = shadow->paddingTop() + shadow->paddingBottom();
    }
    m_client->setWidth(width() - m_decoration->borderLeft() - m_decoration->borderRight() - widthOffset);
    m_client->setHeight(height() - m_decoration->borderTop() - m_decoration->borderBottom() - heightOffset);
}

DecorationShadow *PreviewItem::shadow() const
{
    if (!m_decoration) {
        return nullptr;
    }
    const auto &s = m_decoration->shadow();
    if (!s) {
        return nullptr;
    }
    return s.data();
}

}
}
