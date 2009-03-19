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


#ifndef KIM_SVGSCRIPTLAYOUT_H
#define KIM_SVGSCRIPTLAYOUT_H

#include "kimsvglayout.h"
#include "kimsvgscriptitem.h"
#include "kimsvgscriptwrapper.h"

#include <plasma/svg.h>
#include <KSvgRenderer>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>

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
        virtual QSizeF elementSize(const QString &elem=QString()) const
        {
            return elementRect(elem).size();
        }

        virtual void paint(QPainter *painter,const QRectF &bounds = QRectF(),const QString &elementID=QString());

    public Q_SLOTS:
        void themeUpdated();

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
#endif // KIM_SVGSCRIPTLAYOUT_H
