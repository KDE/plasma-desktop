#include "kimframesvglayout.h"
#include "kimsvglayout.h"

#include <plasma/theme.h>
#include <KDebug>

namespace KIM
{
    FrameSvgLayout::FrameSvgLayout(QObject *parent)
        :SvgLayout(parent)
    {

    }

    FrameSvgLayout::~FrameSvgLayout()
    {

    }

    void FrameSvgLayout::doLayout(const QSizeF &sizeHint,const QString &elem)
    {
        if (elem.isEmpty()) {
            m_sizes["root"] = sizeHint;
            m_rects["root"] = QRectF(QPointF(0,0),sizeHint);
            m_sizes["left"] = render()->boundsOnElement("left").size();
            m_sizes["top"] = render()->boundsOnElement("top").size();
            m_sizes["right"] = render()->boundsOnElement("right").size();
            m_sizes["bottom"] = render()->boundsOnElement("bottom").size();
            m_sizes["topleft"] = QSizeF(m_sizes["left"].width(),m_sizes["top"].height());
            m_sizes["topright"] = QSizeF(m_sizes["right"].width(),m_sizes["top"].height());
            m_sizes["bottomleft"] = QSizeF(m_sizes["left"].width(),m_sizes["bottom"].height());
            m_sizes["bottomright"] = QSizeF(m_sizes["right"].width(),m_sizes["bottom"].height());
            kDebug() << "Size:" << m_sizes["left"] << m_sizes["top"] << m_sizes["right"] << m_sizes["bottom"];
            m_rects["topleft"] = QRectF(0,0,m_sizes["topleft"].width(),m_sizes["topleft"].height());
            m_rects["topright"] = QRectF(sizeHint.width()-m_sizes["topright"].width(),
                    0,m_sizes["topright"].width(),m_sizes["topright"].height());
            m_rects["bottomleft"] = QRectF(0,
                    sizeHint.height()-m_sizes["bottomleft"].height(),
                    m_sizes["bottomleft"].width(),
                    m_sizes["bottomleft"].height());
            m_rects["bottomright"] = QRectF(sizeHint.width()-m_sizes["bottomright"].width(),
                    sizeHint.height()-m_sizes["bottomright"].height(),
                    m_sizes["bottomright"].width(),
                    m_sizes["bottomright"].height());
            m_rects["left"] = QRectF(0,
                    m_sizes["topleft"].height(),
                    m_sizes["left"].width(),
                    sizeHint.height()-m_sizes["topleft"].height()-m_sizes["bottomleft"].height());
            m_rects["right"] = QRectF(sizeHint.width()-m_sizes["right"].width(),
                    m_sizes["topright"].height(),
                    m_sizes["right"].width(),
                    sizeHint.height()-m_sizes["topright"].height()-m_sizes["bottomright"].height());
            m_rects["top"] = QRectF(m_sizes["topleft"].width(),
                    0,
                    sizeHint.width()-m_sizes["topleft"].width()-m_sizes["topright"].width(),
                    m_sizes["top"].height());
            m_rects["bottom"] = QRectF(m_sizes["bottomleft"].width(),
                sizeHint.height()-m_sizes["bottom"].height(),
                sizeHint.width()-m_sizes["bottomleft"].width()-m_sizes["bottomright"].width(),
                m_sizes["bottom"].height());
            m_rects["center"] = QRectF(m_sizes["left"].width(),
                m_sizes["top"].height(),
                sizeHint.width()-m_sizes["left"].width()-m_sizes["right"].width(),
                sizeHint.height()-m_sizes["top"].height()-m_sizes["bottom"].height());
            kDebug() << "Rect:"<<m_rects["left"]<<m_rects["top"]<<m_rects["right"]<<m_rects["bottom"]<<m_rects["center"] << m_rects["topleft"] << m_rects["topright"] << m_rects["bottomleft"] << m_rects["bottomright"];
        }
    }
    
    QRectF FrameSvgLayout::elementRect(const QString &elem) const
    {
        QString key = elem.isEmpty() ? "root" : elem;
        return m_rects[key];
    }

    void FrameSvgLayout::paint(QPainter *painter,const QRectF &bounds,const QString &elementID)
    {
        if (!elementID.isEmpty()) {
            render()->render(painter,elementID,bounds);
        } else {
            render()->render(painter,"topleft",m_rects["topleft"].translated(bounds.topLeft()));
            render()->render(painter,"topright",m_rects["topright"].translated(bounds.topLeft()));
            render()->render(painter,"bottomleft",m_rects["bottomleft"].translated(bounds.topLeft()));
            render()->render(painter,"bottomright",m_rects["bottomright"].translated(bounds.topLeft()));
            render()->render(painter,"left",m_rects["left"].translated(bounds.topLeft()));
            render()->render(painter,"top",m_rects["top"].translated(bounds.topLeft()));
            render()->render(painter,"right",m_rects["right"].translated(bounds.topLeft()));
            render()->render(painter,"bottom",m_rects["bottom"].translated(bounds.topLeft()));
            render()->render(painter,"center",m_rects["center"].translated(bounds.topLeft()));
        }
    }
} // namespace KIM

#include "kimframesvglayout.moc"
// vim: sw=4 sts=4 et tw=100
