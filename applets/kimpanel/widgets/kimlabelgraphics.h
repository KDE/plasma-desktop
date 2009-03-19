#ifndef WIDGETS_KIMLABELGRAPHICS_H
#define WIDGETS_KIMLABELGRAPHICS_H

#include "paintutils.h"
#include <QtGui>

class KIMLabelGraphics:public QGraphicsWidget
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

    KIMLabelGraphics(KIM::RenderType type = KIM::Auxilary, QGraphicsItem *parent = 0);
    ~KIMLabelGraphics();

    void enableHoverEffect(bool b)
    {
        m_hoverEffect = b;
        //generatePixmap();
    }
    bool hasHoverEffect() const
    {
        return m_hoverEffect;
    }

    void setDrawCursor(bool to_draw);
    bool willDrawCursor() const
    {
        return m_drawCursor;
    }
    void setCursorPos(int pos);
    int cursorPos() const
    {
        return m_cursorPos;
    }

    void setTextRenderType(KIM::RenderType type)
    {
        m_renderType = type;
        generatePixmap();
    }
    KIM::RenderType textRenderType() const
    {
        return m_renderType;
    }

    void setText(const QString &text);
    QString text() const
    {
        return m_text;
    }

    void setLabel(const QString &label);
    QString label() const
    {
        return m_label;
    }
    
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint=QSizeF()) const
    {
        if ((which == Qt::MinimumSize) || (which == Qt::PreferredSize)) {
            if (m_pixmap.isNull()) {
                return QSizeF(0,0);
            } else {
                return m_pixmap.size();
            }
        }
        return QGraphicsWidget::sizeHint(which,constraint);
    }

Q_SIGNALS:
    void pressed(bool);
    void clicked();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private Q_SLOTS:
    void generatePixmap();
    void hoverEffect(bool);

private:
    bool m_hoverEffect;
    bool m_drawCursor;
    int m_cursorPos;
    KIM::RenderType m_renderType;
    QString m_text;
    QString m_label;
    QPixmap m_pixmap;
    QPixmap m_reversedPixmap;

    LabelStates m_states;
    QPointF m_clickStartPos;
};
#endif // WIDGETS_KIMLABELGRAPHICS_H
