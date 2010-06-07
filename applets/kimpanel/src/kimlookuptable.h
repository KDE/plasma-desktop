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
#ifndef KIMLOOKUPTABLE_H
#define KIMLOOKUPTABLE_H

#include "kimpanelruntime_export.h"
#include <plasma/theme.h>
#include <plasma/svg.h>
#include <plasma/framesvg.h>
#include <Plasma/Corona>

#include <QWidget>
#include <QHBoxLayout>
#include <QPoint>
#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QEvent>
#include <QMouseEvent>

#include "kimagenttype.h"

class PanelAgent;
class KIMLookupTableGraphics;

class KIMPANELRUNTIME_EXPORT KIMLookupTable: public QWidget
{
Q_OBJECT
public:
    explicit KIMLookupTable(PanelAgent *agent = 0, Plasma::Corona *corona=0, QWidget *parent=0);
    ~KIMLookupTable();
    
Q_SIGNALS:
    
public Q_SLOTS:
    void updateSpotLocation(int x,int y);

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
    bool event(QEvent *e);

private Q_SLOTS:
    void propagateSizeChange();
    void propagateVisibleChange(bool);
    void themeUpdated();

private:
    Plasma::FrameSvg *m_background;

    QHBoxLayout *m_layout;

    bool m_dragging;
    QPoint m_init_pos;

    Plasma::Corona *m_scene;
    QGraphicsView *m_view;
    KIMLookupTableGraphics *m_widget;

    QDesktopWidget m_desktop;

    PanelAgent *m_panel_agent;

    bool m_visible;
};

#endif // KIMLOOKUPTABLE_H
