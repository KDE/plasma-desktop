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

#include "kimsvgscriptwrapper.h"
#include <KDebug>

namespace KIM
{

    SvgScriptWrapper::SvgScriptWrapper(KSvgRenderer *render,QScriptEngine *engine,QObject *parent)
        :QObject(parent),
         m_render(render),
         m_engine(engine)
    {

    }
    
    QScriptValue SvgScriptWrapper::Element(const QString &elemId)
    {
        QString elem = elemId;
        if (elem.isEmpty()) {
            elem = "root";
        }
        if (m_items.contains(elem)) {
            QScriptValue val = m_engine->newQObject(m_items[elem]);
            return val;
        } else {
            QSizeF size;
            if (m_render->elementExists(elem)) {
                size = m_render->boundsOnElement(elem).size();
            } else {
                if (m_render->elementExists("south-"+elem)) {
                    elem = "south-"+elem;
                    size = m_render->boundsOnElement(elem).size();
                } else {
                    size = QSizeF(0,0);
                }
                //size = QSizeF(0,0);
            }
            SvgScriptItem *item = new SvgScriptItem(elem,size);
            m_items[elem] = item;
            QScriptValue val = m_engine->newQObject(item);
            return val;
        }
    }

    void SvgScriptWrapper::reset()
    {
        qDeleteAll(m_items);
        m_items.clear();
    }
} // namespace KIM

#include "kimsvgscriptwrapper.moc"
// vim: sw=4 sts=4 et tw=100
