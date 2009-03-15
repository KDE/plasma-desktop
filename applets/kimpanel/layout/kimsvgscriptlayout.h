
#ifndef WIDGETS_KIM_SCRIPT_SVG_LAYOUT_H
#define WIDGETS_KIM_SCRIPT_SVG_LAYOUT_H

#include "kimsvglayout.h"
#include "kimsvgscriptitem.h"
#include "kimsvgscriptwrapper.h"

#include <plasma/svg.h>
#include <KSvgRenderer>
#include <QtCore>
#include <QtGui>
#include <QtScript>

namespace KIM
{
    //class SvgScriptItem;
    class SvgScriptLayout:public SvgLayout
    {
    Q_OBJECT
    public:
        SvgScriptLayout(QObject *parent = 0);
        virtual ~SvgScriptLayout();

        void setScript(const QString &file);
        void setScript(const QByteArray &data);

        virtual void doLayout(const QSizeF &sizeHint,const QString &elem = QString());

        virtual QRectF elementRect(const QString &elem=QString()) const;

        virtual void paint(QPainter *painter,const QRectF &bounds = QRectF(),const QString &elementID=QString());
    private Q_SLOTS:
        void updateLayers();

    private:
        QString m_path;
        QMap<int,QStringList> m_layers;
        QMap<QString,SvgScriptItem *> m_scriptItems;

        SvgScriptWrapper *m_wrapper;

        QByteArray m_layoutScript;
        QScriptEngine *m_engine;
        QScriptValue m_scriptDoLayout;
    };
} // namespace KIM
#endif // WIDGETS_KIM_SCRIPT_SVG_LAYOUT_H
