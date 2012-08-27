/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
 *   Copyright (C) 2011 by CSSlayer <wengxt@gmail.com>                     *
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

#include "kimpanellabelgraphics.h"

// Qt
#include <QGraphicsSceneMouseEvent>

// KDE
#include <KWindowSystem>

// Plasma
#include <Plasma/Theme>
#include <Plasma/Animator>
#include <Plasma/Animation>
#include <Plasma/PaintUtils>

KimpanelLabelGraphics::KimpanelLabelGraphics(RenderType type, QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      m_drawCursor(false),
      m_cursorPos(0),
      m_renderType(type),
      m_highlight(false)
{
    // Text inside this label will rapidly changed
    setCacheMode(QGraphicsItem::NoCache);
    setAcceptHoverEvents(true);
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(generatePixmap()));
    connect(this, SIGNAL(visibleChanged()), this, SLOT(updateSize()));

    setMinimumSize(0, 0);
    setMaximumSize(0, 0);
}

KimpanelLabelGraphics::~KimpanelLabelGraphics()
{

}


void KimpanelLabelGraphics::mousePressEvent(QGraphicsSceneMouseEvent *event)
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
}

void KimpanelLabelGraphics::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
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

void KimpanelLabelGraphics::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
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

void KimpanelLabelGraphics::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    hoverEffect(true);
    QGraphicsWidget::hoverEnterEvent(event);
}

void KimpanelLabelGraphics::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    hoverEffect(false);
    QGraphicsWidget::hoverLeaveEvent(event);
}

void KimpanelLabelGraphics::hoverEffect(bool show)
{
    LabelStates oldstate = m_states;
    if (show)
        m_states |= HoverState;
    else
        m_states &= ~HoverState; // Will be set once progress is zero again ...

    if (oldstate != m_states)
        update();
}

void KimpanelLabelGraphics::setDrawCursor(bool to_draw)
{
    m_drawCursor = to_draw;
}

void KimpanelLabelGraphics::setCursorPos(int pos)
{
    m_cursorPos = pos;
    generatePixmap();
}

void KimpanelLabelGraphics::setText(const QString& label, const QString& text)
{
    if (label != m_label || m_text != text) {
        m_label = label;
        m_text = text;
        generatePixmap();
    }
}

void KimpanelLabelGraphics::updateSize()
{
    QSizeF sz = minimumSize();
    if (isVisible()) {
        setMinimumSize(m_pixmap.size());
        setMaximumSize(m_pixmap.size());
    }
    else {
        setMinimumSize(0, 0);
        setMaximumSize(0, 0);
    }
    if (sz != minimumSize())
        emit sizeChanged();
}

void KimpanelLabelGraphics::setHighLight(bool highlight)
{
    if (highlight != m_highlight) {
        m_highlight = highlight;
        update();
    }
}

void KimpanelLabelGraphics::setTextRenderType(RenderType type)
{
    m_renderType = type;
}


void KimpanelLabelGraphics::generatePixmap()
{
    QPixmap text_pixmap;
    QPixmap textReversed_pixmap;
    QPixmap label_pixmap;
    QPixmap labelReversed_pixmap;

    QSize size(0, 0);
    if (m_text.isEmpty() && m_label.isEmpty()) {
        m_pixmap = QPixmap();
        m_reversedPixmap = QPixmap();
    } else {
        if (!m_text.isEmpty()) {
            text_pixmap = renderText(m_text, m_renderType, m_drawCursor, m_cursorPos);
            textReversed_pixmap = renderText(m_text, TableLabel, m_drawCursor, m_cursorPos);
            size.setWidth(size.width() + text_pixmap.width());
            size.setHeight(text_pixmap.height());
        }
        if (!m_label.isEmpty()) {
            labelReversed_pixmap = renderText(m_label, TableEntry);
            label_pixmap = renderText(m_label, TableLabel);
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
            p.drawPixmap(0, 0, label_pixmap);
            p1.drawPixmap(0, 0, labelReversed_pixmap);
            if (!text_pixmap.isNull()) {
                p.drawPixmap(label_pixmap.width(), 0, text_pixmap);
                p1.drawPixmap(label_pixmap.width(), 0, textReversed_pixmap);
            }
        } else {
            p.drawPixmap(0, 0, text_pixmap);
            p1.drawPixmap(0, 0, textReversed_pixmap);
        }
    }

    updateSize();
}

void KimpanelLabelGraphics::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    int h_spacing = (boundingRect().height() - m_pixmap.height()) / 2;

    if (Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).value() < 128
            && KWindowSystem::compositingActive()
       ) {
        QRectF haloRect = m_pixmap.rect().translated(9, 3).translated(0, h_spacing);
        if (haloRect.width() > 18 && haloRect.height() > 6) {
            haloRect.setWidth(haloRect.width() - 18);
            haloRect.setHeight(haloRect.height() - 6);
            Plasma::PaintUtils::drawHalo(painter, haloRect);
        }
    }
    if (m_renderType == TableEntry
        && !m_label.isEmpty()
        && (m_highlight || (m_states & HoverState))
    ) {
        painter->drawPixmap(0, h_spacing,
                            m_reversedPixmap);
    } else {
        painter->drawPixmap(0, h_spacing,
                            m_pixmap);
    }
}

