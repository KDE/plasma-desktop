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

#ifndef KIM_FRAME_SVG_LAYOUT_H
#define KIM_FRAME_SVG_LAYOUT_H

#include "kimsvglayout.h"

#include <plasma/svg.h>
#include <KSvgRenderer>

namespace KIM
{
    class KDE_EXPORT FrameSvgLayout:public SvgLayout
    {
    Q_OBJECT
    public:
        explicit FrameSvgLayout(QObject *parent = 0);
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
#endif // KIM_FRAME_SVG_LAYOUT_H
