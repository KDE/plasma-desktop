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

#ifndef KIM_SVG_LAYOUT_H
#define KIM_SVG_LAYOUT_H

#include <plasma/svg.h>
#include <plasma/theme.h>
#include <KSvgRenderer>
#include <QtSvg>

namespace KIM
{
    class KDE_EXPORT SvgLayout:public QObject
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

    public Q_SLOTS:
        void themeUpdated();

    private:
        KSvgRenderer *m_renderer;
        bool m_themed;
        QString m_path;
        QString m_themePath;
    };
} // namespace KIM
#endif // KIM_SVG_LAYOUT_H
