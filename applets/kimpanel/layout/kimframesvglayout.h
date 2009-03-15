#ifndef WIDGETS_KIM_FRAME_SVG_LAYOUT_H
#define WIDGETS_KIM_FRAME_SVG_LAYOUT_H

#include "kimsvglayout.h"

#include <plasma/svg.h>
#include <KSvgRenderer>
#include <QtCore>
#include <QtGui>

namespace KIM
{
    class FrameSvgLayout:public SvgLayout
    {
    Q_OBJECT
    public:
        FrameSvgLayout(QObject *parent = 0);
        virtual ~FrameSvgLayout();

        virtual void doLayout(const QSizeF &sizeHint,const QString &elem = QString());

        virtual QRectF elementRect(const QString &elem=QString()) const;

        virtual void paint(QPainter *painter,const QRectF &bounds = QRectF(),const QString &elementID=QString());

    private:
        KSvgRenderer m_renderer;
        QString m_path;
        QMap<QString,QRectF> m_rects;
        QMap<QString,QSizeF> m_sizes;
    };
} // namespace KIM
#endif // WIDGETS_KIM_FRAME_SVG_LAYOUT_H
