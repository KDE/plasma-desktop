/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
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

#include "kimlabelgraphics.h"

#include <Plasma/Theme>
#include <Plasma/Animator>
#include <Plasma/Animation>

#include <QGraphicsSceneMouseEvent>

KIMLabelGraphics::KIMLabelGraphics(KIM::RenderType type,QGraphicsItem *parent)
    :QGraphicsWidget(parent),
     m_hoverEffect(false),
     m_drawCursor(false),
     m_cursorPos(0),
     m_renderType(type)
{
    setAcceptHoverEvents(true);
    connect(Plasma::Theme::defaultTheme(),SIGNAL(themeChanged()),
            this,SLOT(generatePixmap()));
}

KIMLabelGraphics::~KIMLabelGraphics()
{

}

void KIMLabelGraphics::setDrawCursor(bool to_draw)
{
    m_drawCursor = to_draw;
    generatePixmap();
}

void KIMLabelGraphics::setCursorPos(int pos)
{
    if (m_drawCursor) {
        m_cursorPos = pos;
        generatePixmap();
    }
}

void KIMLabelGraphics::setText(const QString &text)
{
    m_text = text;
    generatePixmap();
}

void KIMLabelGraphics::setLabel(const QString &label)
{
    m_label = label;
    generatePixmap();
}

void KIMLabelGraphics::generatePixmap()
{
    QSize old_size = m_pixmap.size();

    QPixmap text_pixmap;
    QPixmap textReversed_pixmap;
    QPixmap label_pixmap;
    QPixmap labelReversed_pixmap;
    QPixmap cursor_pixmap;

    QString text;

    QSize size(0,0);

    if (m_drawCursor) {
        text = m_text.left(m_cursorPos) + 'I' + m_text.mid(m_cursorPos);
    } else {
        text = m_text;
    }

    if (text.isEmpty() && m_label.isEmpty()) {
        m_pixmap = QPixmap();
        m_reversedPixmap = QPixmap();
    } else {
        if (!text.isEmpty()) {
            text_pixmap = KIM::renderText(text,m_renderType);
            textReversed_pixmap = KIM::renderText(text,KIM::TableLabel);
            size.setWidth(size.width() + text_pixmap.width());
            size.setHeight(text_pixmap.height());
        }
        if (!m_label.isEmpty()) {
            labelReversed_pixmap = KIM::renderText(m_label,KIM::TableEntry);
            label_pixmap = KIM::renderText(m_label,KIM::TableLabel);
            size.setWidth(size.width() + label_pixmap.width());
            size.setHeight(label_pixmap.height());
        }
        m_pixmap = QPixmap(size);
        m_reversedPixmap = QPixmap(size);
        m_pixmap.fill(Qt::transparent);
        m_reversedPixmap.fill(Qt::transparent);
        QPainter p(&m_pixmap);
        QPainter p1(&m_reversedPixmap);
        if (!label_pixmap.isNull()) {
            p.drawPixmap(0,0,label_pixmap);
            p1.drawPixmap(0,0,labelReversed_pixmap);
            if (!text_pixmap.isNull()) {
                p.drawPixmap(label_pixmap.width(),0,text_pixmap);
                p1.drawPixmap(label_pixmap.width(),0,textReversed_pixmap);
            }
        } else {
            p.drawPixmap(0,0,text_pixmap);
            p1.drawPixmap(0,0,textReversed_pixmap);
        }
        //kDebug() << m_pixmap.size();
    }

    if (m_pixmap.size() != old_size) {
        updateGeometry();
    }
}

void KIMLabelGraphics::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    int h_spacing = (boundingRect().height() - m_pixmap.height())/2;
    if (m_hoverEffect && (m_states & HoverState)) {
        painter->drawPixmap(0,h_spacing,
            m_reversedPixmap);
    } else {
        painter->drawPixmap(0,h_spacing,
            m_pixmap);
    }
}

void KIMLabelGraphics::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QGraphicsWidget::mousePressEvent(event);
        return;
    }

    m_states |= PressedState;
    m_clickStartPos = scenePos();

    if (boundingRect().contains(event->pos())) {
        emit pressed(true);
    }

//    update();
}

void KIMLabelGraphics::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (~m_states & PressedState) {
        QGraphicsWidget::mouseMoveEvent(event);
        return;
    }

    if (boundingRect().contains(event->pos())) {
        if (~m_states & HoverState) {
            m_states |= HoverState;
            update();
        }
    } else {
        if (m_states & HoverState) {
            m_states &= ~HoverState;
            update();
        }
    }
}

void KIMLabelGraphics::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (~m_states & PressedState) {
        QGraphicsWidget::mouseMoveEvent(event);
        return;
    }

    m_states &= ~PressedState;

    //don't pass click when the mouse was moved
    bool handled = m_clickStartPos != scenePos();
    if (!handled) {
        if (boundingRect().contains(event->pos())) {
            emit clicked();
        }
        emit pressed(false);
    }

    update();
}

void KIMLabelGraphics::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    hoverEffect(true);
    QGraphicsWidget::hoverEnterEvent(event);
}

void KIMLabelGraphics::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_states &= ~HoverState; // Will be set once progress is zero again ...
    hoverEffect(false);
    QGraphicsWidget::hoverLeaveEvent(event);
}

void KIMLabelGraphics::hoverEffect(bool show)
{
    if (show) {
        m_states |= HoverState;
    }

    Plasma::Animation *animation = m_animation.data();
    if (!animation) {
        animation = Plasma::Animator::create(Plasma::Animator::FadeAnimation);
        animation->setTargetWidget(this);
        animation->setProperty("duration", 150);
        animation->setProperty("startValue", 0.0);
        animation->setProperty("endValue", 1.0);
        m_animation = animation;
    } else if (animation->state() == QAbstractAnimation::Running) {
        animation->pause();
    }

    if (show) {
        animation->setProperty("easingCurve", QEasingCurve::InQuad);
        animation->setProperty("direction", QAbstractAnimation::Forward);
        animation->start(QAbstractAnimation::KeepWhenStopped);
    } else {
        animation->setProperty("easingCurve", QEasingCurve::OutQuad);
        animation->setProperty("direction", QAbstractAnimation::Backward);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

#include "kimlabelgraphics.moc"
// vim: sw=4 sts=4 et tw=100
