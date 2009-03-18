#ifndef LAYOUT_KIMSVGSCRIPTWRAPPER_H
#define LAYOUT_KIMSVGSCRIPTWRAPPER_H

#include <KSvgRenderer>
#include <QtCore>
#include <QtScript>
#include "kimsvgscriptitem.h"

namespace KIM
{
    class SvgScriptWrapper:public QObject
    {
        Q_OBJECT
    public:
        SvgScriptWrapper(KSvgRenderer *render,QScriptEngine *engine,QObject *parent=0);

        ~SvgScriptWrapper()
        {

        }

        QMap<QString,SvgScriptItem *> items() const
        {
            return m_items;
        }

    public Q_SLOTS:
        QScriptValue Element(const QString &elem);
        void reset();

    private:
        KSvgRenderer *m_render;
        QScriptEngine *m_engine;
        QMap<QString,SvgScriptItem *> m_items;
    };
}
#endif // LAYOUT_KIMSVGSCRIPTWRAPPER_H
