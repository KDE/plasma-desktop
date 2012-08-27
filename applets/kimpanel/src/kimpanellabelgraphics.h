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

#ifndef KIMPANEL_LABELGRAPHICS_H
#define KIMPANEL_LABELGRAPHICS_H

#include "paintutils.h"

// Qt
#include <QGraphicsWidget>

class KimpanelLabelGraphics: public QGraphicsWidget
{
    Q_OBJECT
public:
    enum LabelState {
        NoState = 0,
        HoverState = 1,
        PressedState = 2,
        ManualPressedState = 4
    };
    Q_DECLARE_FLAGS(LabelStates, LabelState)

    explicit KimpanelLabelGraphics(RenderType type = Auxiliary, QGraphicsItem *parent = 0);
    ~KimpanelLabelGraphics();

    void setDrawCursor(bool to_draw);
    void setCursorPos(int pos);
    void setTextRenderType(RenderType type);
    void setText(const QString &label, const QString &text);
    void setHighLight(bool highlight);

Q_SIGNALS:
    void pressed(bool);
    void clicked();
    void lookupTablePageUp();
    void lookupTablePageDown();
    void selectCandidate();
    void sizeChanged();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
private Q_SLOTS:
    void generatePixmap();
    void updateSize();

private:
    void hoverEffect(bool show);
    bool m_drawCursor;
    int m_cursorPos;
    RenderType m_renderType;
    QString m_text;
    QString m_label;
    QPixmap m_pixmap;
    QPixmap m_reversedPixmap;
    QPointF m_clickStartPos;
    LabelStates m_states;
    bool m_highlight;
};

#endif // KIMPANEL_LABELGRAPHICS_H
