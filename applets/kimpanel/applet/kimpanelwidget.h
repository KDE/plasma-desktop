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
#ifndef KIMPANELWIDGET_H
#define KIMPANELWIDGET_H

#include <plasma/applet.h>
#include <plasma/widgets/iconwidget.h>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QGraphicsLayoutItem>
#include <QList>
#include <KIcon>
#include <KDialog>

#include "kimpaneltype.h"
#include "kimpanelagent.h"
#include "lookuptablewidget.h"
#include "statusbarwidget.h"
#include "kimpanellayout.h"

class KIMPanelWidget : public QGraphicsWidget
{
Q_OBJECT
public:
    KIMPanelWidget(QGraphicsItem *parent);
    ~KIMPanelWidget();

    /**
     * Returns hints about the geometry of the figure
     * @return Hints about proportionality of the applet
     */
//X      QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;

    inline int iconCount() const
    {
        return m_icons.size();
    }

    bool collapsible() const 
    {
        return m_enableCollapse;
    }
    void setCollapsible(bool b); 

    QList<QAction *> contextualActions() const;


Q_SIGNALS:
    void collapsed(bool b);
    void iconCountChanged(int iconCount);

    void triggerProperty(const QString &key);

public Q_SLOTS:

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected Q_SLOTS:

private Q_SLOTS:
    void updateProperty(const Property &prop);
    void registerProperties(const QList<Property> &props);
    void execDialog(const Property &prop);
    void execMenu(const QList<Property> &prop_list);

    void changeCollapseStatus();

private:
    KIMPanelLayout *m_layout;
    QList<Plasma::IconWidget *> m_icons;
    Plasma::FrameSvg *m_background;
    Plasma::IconWidget *m_arrow;

    bool m_collapsed;
    bool m_enableCollapse;
    QAction *m_collapseAction;
    Plasma::IconWidget *m_collapseIcon;

    PanelAgent *m_panel_agent;

    QList<Property> m_props;
    QMap<QString,Plasma::IconWidget *> m_prop_map;
    QSignalMapper m_icon_mapper;

    LookupTableWidget *m_lookup_table;
    StatusBarWidget *m_statusbar;
    QList<QAction *> m_statusbarActions;

    bool m_empty;
};
#endif // KIMPANELWIDGET_H
