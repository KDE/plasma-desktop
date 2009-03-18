#include "kimsvgscriptlayout.h"
#include "kimsvgscriptitem.h"
#include "kimsvgscriptwrapper.h"

#include <plasma/theme.h>
#include <KSvgRenderer>
#include <KDebug>
#include <QtCore>

namespace KIM
{
    SvgScriptLayout::SvgScriptLayout(QObject *parent)
        :SvgLayout(parent),
         m_engine(new QScriptEngine())
    {
        m_wrapper = new SvgScriptWrapper(render(),m_engine,this);
        m_engine->globalObject().setProperty("Svg",m_engine->newQObject(m_wrapper));
    }

    SvgScriptLayout::~SvgScriptLayout()
    {

    }

    void SvgScriptLayout::setScript(const QString &path)
    {
        QFile file(path);
        if (!file.exists() || !file.open(QIODevice::ReadOnly))
            return;
        setScript(file.readAll());
    }

    void SvgScriptLayout::setScript(const QByteArray &data)
    {
        m_layoutScript = data;
        m_engine->evaluate(data);
        m_scriptDoLayout = m_engine->globalObject().property("do_layout");
        if (m_engine->hasUncaughtException()) {
            kDebug() << "Error in evaluating script:"<<m_engine->uncaughtExceptionLineNumber();
            foreach (const QString &err, m_engine->uncaughtExceptionBacktrace()) {
                kDebug() << err;
            }
        }
    }

    void SvgScriptLayout::doLayout(const QSizeF &sizeHint,const QString &elem)
    {
        QString key = elem;
        if (key.isEmpty()) {
            key = "root";
        }
        m_scriptDoLayout.call(QScriptValue(),
                QScriptValueList() << sizeHint.width() << sizeHint.height() << key);
        if (m_engine->hasUncaughtException()) {
            kDebug() << "Error in evaluating do_layout:" << m_engine->uncaughtExceptionLineNumber();
            foreach (const QString &err, m_engine->uncaughtExceptionBacktrace()) {
                kDebug() << err;
            }
        }
        m_scriptItems = m_wrapper->items();
    }

    QRectF SvgScriptLayout::elementRect(const QString &elem) const
    {
        QString key = elem.isEmpty() ? "root" : elem;
        if (m_scriptItems.contains(key)) {
            SvgScriptItem *item = m_scriptItems[key];
            return QRectF(item->x(),item->y(),
                    item->width(),item->height());
        } else {
            return QRectF();
        }
    }

    void SvgScriptLayout::themeUpdated()
    {
        m_wrapper->reset();
        m_scriptItems.clear();
        m_layers.clear();
    }

    void SvgScriptLayout::updateLayers()
    {
        m_layers.clear();
        foreach (const QString &elem, m_scriptItems.keys()) {
            SvgScriptItem *item = m_scriptItems[elem];
            if (!item || 
                    qFuzzyCompare(item->width()+1,1.0) || 
                    qFuzzyCompare(item->height()+1,1.0) || 
                    !render()->elementExists(elem)) {

                continue;
            }
            m_layers[item->layer()] << elem;
        }
    }

    void SvgScriptLayout::paint(QPainter *painter,const QRectF &bounds,const QString &elementID)
    {
        updateLayers();
        painter->save();
        foreach (int layer, m_layers.keys()) {
            foreach (const QString &elem, m_layers[layer]) {
#if 0
                if (elem == "root") {
                    continue;
                }
#endif
                SvgScriptItem *item = m_scriptItems[elem];
                QRectF r(item->x(),item->y(),item->width(),item->height());
                r.translate(bounds.topLeft());
                render()->render(painter,elem,r);
            }
        }
        painter->restore();
    }
} // namespace KIM

#include "kimsvgscriptlayout.moc"
// vim: sw=4 sts=4 et tw=100
