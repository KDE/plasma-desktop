#ifndef KIMSVGITEMSCRIPT_H
#define KIMSVGITEMSCRIPT_H

#include <KDebug>
#include <QtCore>

namespace KIM
{
    class SvgScriptItem:public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QString element READ element)
        Q_PROPERTY(qreal x READ x WRITE setX)
        Q_PROPERTY(qreal y READ y WRITE setY)
        Q_PROPERTY(qreal width READ width WRITE setWidth)
        Q_PROPERTY(qreal height READ height WRITE setHeight)
        Q_PROPERTY(int layer READ layer WRITE setLayer)

        Q_PROPERTY(qreal defaultWidth READ defaultWidth)
        Q_PROPERTY(qreal defaultHeight READ defaultHeight)

    public:
        SvgScriptItem(const QString &elem,const QSizeF &defaultSize, QObject *parent = 0)
            :QObject(parent),
            m_element(elem),
            m_x(.0),
            m_y(.0),
            m_w(defaultSize.width()),
            m_h(defaultSize.height()),
            m_defaultW(defaultSize.width()),
            m_defaultH(defaultSize.height()),
            m_layer(0)
        {
        }
        ~SvgScriptItem()
        {

        }

    public:
        qreal x() const
        {
            return m_x;
        }
        void setX(qreal x)
        {
            m_x = x;
        }
        qreal y() const
        {
            return m_y;
        }
        void setY(qreal y)
        {
            m_y = y;
        }
        qreal width() const
        {
            return m_w;
        }
        void setWidth(qreal w)
        {
            m_w = w;
        }
        qreal height() const
        {
            return m_h;
        }
        void setHeight(qreal h)
        {
            m_h = h;
        }

        qreal defaultWidth() const
        {
            return m_defaultW;
        }
        void setDefaultWidth(qreal defaultW)
        {
            m_defaultW = defaultW;
        }
        qreal defaultHeight() const
        {
            return m_defaultH;
        }
        void setDefaultHeight(qreal defaultH)
        {
            m_defaultH = defaultH;
        }

        int layer() const
        {
            return m_layer;
        }
        void setLayer(int layer)
        {
            m_layer = layer;
        }

        QString element() const
        {
            return m_element;
        }
        void setElement(const QString &elem)
        {
            m_element = elem;
        }

    private:
        QString m_element;
        qreal m_x;
        qreal m_y;
        qreal m_w;
        qreal m_h;
        qreal m_defaultW;
        qreal m_defaultH;
        int m_layer;
    };
} // namespace KIM

#endif // KIMSVGITEMSCRIPT_H
