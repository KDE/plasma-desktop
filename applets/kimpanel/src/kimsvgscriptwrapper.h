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

#ifndef KIMSVGSCRIPTWRAPPER_H
#define KIMSVGSCRIPTWRAPPER_H

#include <KSvgRenderer>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>
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
#endif // KIMSVGSCRIPTWRAPPER_H
