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
#ifndef KIM_STATUSBAR_GRAPHICS_H
#define KIM_STATUSBAR_GRAPHICS_H

#include <plasma/applet.h>
#include <plasma/framesvg.h>
#include <plasma/widgets/iconwidget.h>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QGraphicsLayoutItem>
#include <QList>
#include <QSignalMapper>
#include <QSet>
#include "kimpanelruntime_export.h"

//#include "kimagenttype.h"
#include "kimpanelagent.h"
#include "kimpanellayout.h"

class PanelAgent;
class KIMStatusBar;

class KIMPANELRUNTIME_EXPORT KIMStatusBarGraphics : public QGraphicsWidget
{
Q_OBJECT
public:
    explicit KIMStatusBarGraphics(PanelAgent *agent=0, QGraphicsItem *parent=0);
    ~KIMStatusBarGraphics();

    int iconCount() const;

    bool collapsible() const 
    {
        return m_enableCollapse;
    }
    bool logoVisible() const
    {
        return m_logoVisible;
    }
    void setCollapsible(bool b); 
    void showLogo(bool b);

    QList<QAction *> actions() const;


Q_SIGNALS:
    void collapsed(bool b);
    void iconCountChanged();
    void adjustLocation(int x,int y);

    void triggerProperty(const QString &key);

public Q_SLOTS:
    void updateProperty(const Property &prop);
    void registerProperties(const QList<Property> &props);
    void execDialog(const Property &prop);
    void execMenu(const QList<Property> &props);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected Q_SLOTS:

private Q_SLOTS:
    void changeCollapseStatus();
    void hiddenActionToggled();

private:
    KIMPanelLayout *m_layout;
    QList<Plasma::IconWidget *> m_icons;
    Plasma::FrameSvg *m_background;

    bool m_empty;
    bool m_collapsed;
    bool m_enableCollapse;
    bool m_logoVisible;

    QAction *m_collapseAction;
    QAction *m_reloadConfigAction;

    QAction *m_filterAction;
    QMenu *m_filterMenu;
    QSet<QString> m_hiddenProperties;

    Plasma::IconWidget *m_logoIcon;
    Plasma::IconWidget *m_collapseIcon;

    QList<Property> m_props;
    QMap<QString,Plasma::IconWidget *> m_prop_map;
    QSignalMapper *m_icon_mapper;

    QList<QAction *> m_statusbarActions;

    PanelAgent *m_panel_agent;
};
#endif // KIM_STATUSBAR_GRAPHICS_H
