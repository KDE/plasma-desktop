/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
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

#include "kimsvgscriptlayout.h"
#include "kimsvgscriptitem.h"
#include "kimsvgscriptwrapper.h"

#include <plasma/theme.h>
#include <KDebug>
#include <QFile>
#include <QPainter>
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
        Q_UNUSED(elementID);
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
