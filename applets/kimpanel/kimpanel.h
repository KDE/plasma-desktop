/*
 *   Copyright (C) 2007 Wang Hoi <zealot.hoi@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 (or, at
 *   your option, any later version) as published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KIMPANEL_H
#define KIMPANEL_H

#include "plasma/applet.h"
#include "plasma/svg.h"
#include "plasma/dataengine.h"
#include "kicon.h"
#include <QString>
#include <QPainter>

class PanelAgent;
class KIMPanel : public Plasma::Applet {
Q_OBJECT
public:
    KIMPanel( QObject* parent, const QVariantList& args );
    ~KIMPanel();
protected:
    void init();
    void paintInterface(QPainter *painter, 
            const QStyleOptionGraphicsItem *option, 
            const QRect& contentsRect);
private slots:
private:
    Plasma::Svg m_svg;
    KIcon m_icon;
    PanelAgent *m_panel_agent;
};
K_EXPORT_PLASMA_APPLET(kimpanel, KIMPanel)
#endif
