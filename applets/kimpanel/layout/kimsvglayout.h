#ifndef WIDGETS_KIM_SVG_LAYOUT_H
#define WIDGETS_KIM_SVG_LAYOUT_H

#include <plasma/svg.h>
#include <plasma/theme.h>
#include <KSvgRenderer>
#include <QtSvg>

namespace KIM
{
    class SvgLayout:public QObject
    {
    Q_OBJECT
    public:
        explicit SvgLayout(QObject *parent = 0);
        virtual ~SvgLayout();

        virtual void setImagePath(const QString &path);

        virtual KSvgRenderer *render()
        {
            return m_renderer;
        }

        virtual void doLayout(const QSizeF &sizeHint,const QString &elem = QString()) = 0;

        virtual QRectF elementRect(const QString &elem=QString()) const;

        virtual void paint(QPainter *painter,const QRectF &bounds=QRectF(),const QString &elementID=QString()) = 0;

    private:
        KSvgRenderer *m_renderer;
        bool m_themed;
        QString m_path;
        QString m_themePath;
    };
} // namespace KIM
#endif // WIDGETS_KIM_SVG_LAYOUT_H
