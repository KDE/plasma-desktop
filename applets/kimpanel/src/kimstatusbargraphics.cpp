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

#include "kimstatusbargraphics.h"
#include "kimpanelsettings.h"

#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsWidget>
#include <QMenu>

#include <KDebug>
#include <KMessageBox>
#include <KToggleAction>

#include <plasma/containment.h>
#include <plasma/dialog.h>
#include <plasma/corona.h>
#include <plasma/tooltipmanager.h>

#include <math.h>
#include "paintutils.h"
//#include "kimpanelagent.h"

KIMStatusBarGraphics::KIMStatusBarGraphics(PanelAgent *agent, QGraphicsItem *parent)
  : QGraphicsWidget(parent),
    m_layout(0),
    m_empty(true),
    m_collapsed(false),
    m_enableCollapse(false),
    m_logoVisible(false),
    m_icon_mapper(new QSignalMapper(this)),
    m_panel_agent(agent)
{
    m_hiddenProperties = QSet<QString>::fromList(KIM::Settings::self()->statusbarHiddenProperties());

    m_filterAction = new QAction(KIcon("view-filter"),i18n("Icon Filter"),this);
    m_filterMenu = new QMenu;
    m_filterAction->setMenu(m_filterMenu);

    setContentsMargins(0,0,0,0);

    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/panel-background");

//X     m_layout = new QGraphicsGridLayout(this);
    m_layout = new KIMPanelLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    connect(m_icon_mapper,SIGNAL(mapped(const QString &)),
            this,SIGNAL(triggerProperty(const QString &)));

    m_logoIcon = new Plasma::IconWidget(this);
    m_logoIcon->setIcon(KIcon("draw-freehand"));
    m_logoIcon->hide();

    m_collapseAction = new QAction(KIcon("arrow-up-double"),i18n("Expand out"),this);
    connect(m_collapseAction, SIGNAL(triggered()), SLOT(changeCollapseStatus()));
    m_collapseIcon = new Plasma::IconWidget(this);
    m_collapseIcon->setIcon(m_collapseAction->icon());
    connect(m_collapseIcon,SIGNAL(clicked()),m_collapseAction,SIGNAL(triggered()));
    m_collapseIcon->hide();
    Plasma::ToolTipContent data(m_collapseAction->text(),i18n("Expand out from the panel to a floating widget"),m_collapseAction->icon());
    Plasma::ToolTipManager::self()->setContent(m_collapseIcon,data);

    m_reloadConfigAction = new QAction(KIcon("view-refresh"),i18n("Reload Config"),this);
//X     connect(m_reloadConfigAction,SIGNAL(triggered()),agent,SIGNAL(ReloadConfig()));

    // change from true -> false
    //m_collapsed = true;
    //changeCollapseStatus();

    //setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    if (m_panel_agent) {
        connect(m_panel_agent,
            SIGNAL(registerProperties(const QList<Property> &)),
            this,
            SLOT(registerProperties(const QList<Property> &)));
        connect(m_panel_agent,
            SIGNAL(updateProperty(const Property &)),
            this,
            SLOT(updateProperty(const Property &)));
        connect(this,
            SIGNAL(triggerProperty(const QString &)),
            m_panel_agent,
            SIGNAL(TriggerProperty(const QString &)));

        connect(m_panel_agent,
            SIGNAL(execDialog(const Property &)),
            this,
            SLOT(execDialog(const Property &)));
        connect(m_panel_agent,
            SIGNAL(execMenu(const QList<Property> &)),
            this,
            SLOT(execMenu(const QList<Property> &)));

        m_panel_agent->created();
    }
}

KIMStatusBarGraphics::~KIMStatusBarGraphics()
{
    KIM::Settings::self()->setStatusbarHiddenProperties(m_hiddenProperties.toList());

    KIM::Settings::self()->writeConfig();
}

int KIMStatusBarGraphics::iconCount() const
{
    return m_layout->count();
}

void KIMStatusBarGraphics::setCollapsible(bool b)
{
    if (m_enableCollapse != b) {
        m_enableCollapse = b;
        registerProperties(m_props);
    }
}

void KIMStatusBarGraphics::showLogo(bool b)
{
    if (m_logoVisible != b) {
        m_logoVisible = b;
        registerProperties(m_props);
    }
}

QList<QAction *> KIMStatusBarGraphics::actions() const
{
    QList<QAction *> result;

    if (m_enableCollapse) {
        result << m_collapseAction;
    }

    result << m_filterAction;

    return result;
}

void KIMStatusBarGraphics::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void KIMStatusBarGraphics::updateProperty(const Property &prop)
{
    if (!m_prop_map.contains(prop.key)) {
        return;
    }

    if (m_hiddenProperties.contains(prop.key)) {
        return;
    }

    Plasma::IconWidget *prop_icon = m_prop_map.value(prop.key);

    KIcon icon;

    if (!prop.icon.isEmpty()) {
//        icon_pixmap = KIcon(prop.icon).pixmap(KIconLoader::SizeSmall,KIconLoader::SizeSmall,Qt::KeepAspectRatio);
        icon = KIcon(prop.icon);
    } else {
        icon = KIcon(KIM::renderText(prop.label,KIM::Statusbar).scaled(256,256,Qt::KeepAspectRatio));
//        icon = KIcon(renderText(prop.label));
    }

    prop_icon->setIcon(icon);

    Plasma::ToolTipContent data(prop.label,prop.tip,icon);
    Plasma::ToolTipManager::self()->setContent(prop_icon,data);
    //prop_button->setToolTip(prop.tip);
}

void KIMStatusBarGraphics::registerProperties(const QList<Property> &props)
{
    while (m_layout->count()) {
        QGraphicsLayoutItem *item = m_layout->itemAt(0);
        m_layout->removeAt(0);
        item->graphicsItem()->hide();
    }
    
    m_icons.clear();
//X     if (m_logoVisible) {
//X         m_icons << m_logoIcon;
//X     }
    qDeleteAll(m_prop_map);
    m_prop_map.clear();
    m_filterMenu->clear();
    m_props = props;

    Q_FOREACH (const Property &prop, props) {

        KIcon kicon;

        if (!prop.icon.isEmpty()) {
            kicon = KIcon(prop.icon);
        } else {
            kicon = KIcon(KIM::renderText(prop.label,KIM::Statusbar).scaled(256,256,Qt::KeepAspectRatio));
           // kicon = KIcon(renderText(prop.label));
        }

        Plasma::IconWidget *icon = new Plasma::IconWidget(kicon,"",this);
        Plasma::ToolTipContent data(prop.label,prop.tip,kicon);
        Plasma::ToolTipManager::self()->setContent(icon,data);

#if 0
        if (! m_hiddenProperties.contains(prop.key)) {
            m_icons << icon;
        }
#endif
        m_prop_map.insert(prop.key, icon);
//X         m_layout->addItem(icon);
        //QAction *hiddenAction = new KToggleAction(kicon,prop.label,m_filterMenu);
        QAction *hiddenAction = new KToggleAction(prop.label,m_filterMenu);
        hiddenAction->setCheckable(true);
        hiddenAction->setChecked(! m_hiddenProperties.contains(prop.key));
        hiddenAction->setData(prop.key);
        connect(hiddenAction,SIGNAL(toggled(bool)),this,SLOT(hiddenActionToggled()));
        m_filterMenu->addAction(hiddenAction);

        connect(icon,SIGNAL(clicked()),m_icon_mapper,SLOT(map()));
        m_icon_mapper->setMapping(icon,prop.key);
    }
    
    m_icons.clear();

    Q_FOREACH (const Property &prop, m_props) {
        if (! m_hiddenProperties.contains(prop.key)) {
            m_icons << m_prop_map.value(prop.key);
            m_prop_map.value(prop.key)->show();
        } else {
            m_prop_map.value(prop.key)->hide();
        }
    }

    if (m_enableCollapse) {
        QString key = "__collapse";
        QAction *hiddenAction = new KToggleAction(m_collapseAction->text(),m_filterMenu);
        hiddenAction->setCheckable(true);
        hiddenAction->setChecked(! m_hiddenProperties.contains(key));
        hiddenAction->setData(key);
        connect(hiddenAction,SIGNAL(toggled(bool)),this,SLOT(hiddenActionToggled()));
        m_filterMenu->addAction(hiddenAction);

        if (m_hiddenProperties.contains(key)) {
            m_collapseIcon->hide();
        } else {
            m_icons << m_collapseIcon;
            m_collapseIcon->show();
        }
    } else {
        m_collapseIcon->hide();
    }

    m_layout->setItems(m_icons);

    emit iconCountChanged();
}

void KIMStatusBarGraphics::changeCollapseStatus()
{
    m_collapsed = !m_collapsed;
    if (m_collapsed) {
        m_collapseAction->setIcon(KIcon("arrow-down-double"));
        m_collapseAction->setText(i18n("Collapse to panel"));
        m_collapseIcon->setIcon(m_collapseAction->icon());
        Plasma::ToolTipContent data(m_collapseAction->text(),i18n("Embed into the panel"),m_collapseAction->icon());
        Plasma::ToolTipManager::self()->setContent(m_collapseIcon,data);
    } else {
        m_collapseAction->setIcon(KIcon("arrow-up-double"));
        m_collapseAction->setText(i18n("Expand out"));
        m_collapseIcon->setIcon(m_collapseAction->icon());
        Plasma::ToolTipContent data(m_collapseAction->text(),i18n("Expand out from the panel to a floating widget"),m_collapseAction->icon());
        Plasma::ToolTipManager::self()->setContent(m_collapseIcon,data);
    }
    emit collapsed(m_collapsed);
}

void KIMStatusBarGraphics::execDialog(const Property &prop) 
{
    KMessageBox::information(NULL,prop.tip,prop.label);
}

void KIMStatusBarGraphics::execMenu(const QList<Property> &prop_list)
{
    QMenu *menu = new QMenu();
    QAction *action;
    QSignalMapper *mapper = new QSignalMapper(this);
    connect(mapper,SIGNAL(mapped(const QString&)),m_panel_agent,SIGNAL(TriggerProperty(const QString &)));
    foreach (const Property &prop, prop_list) {
        action = new QAction(QIcon(prop.icon),prop.label,menu);
        mapper->setMapping(action,prop.key);
        connect(action,SIGNAL(triggered()),mapper,SLOT(map()));
        menu->addAction(action);
    }
    menu->exec(QCursor::pos());
    delete menu;
}

void KIMStatusBarGraphics::hiddenActionToggled()
{
    QAction *action = qobject_cast<QAction *>(sender());

    if (action) {
        QString key = action->data().toString();

        if (key == "__collapse") {
            if (action->isChecked()) {
                m_collapseIcon->show();

                m_layout->addItem(m_collapseIcon);

                m_hiddenProperties.remove(key);
            } else {
                m_collapseIcon->hide();

                m_layout->removeAt(m_layout->count()-1);

                m_hiddenProperties.insert(key);
            }

            emit iconCountChanged();
            return;
        }

        if (action->isChecked()) {
            m_hiddenProperties.remove(key);
            m_prop_map.value(key)->show();
        } else {
            m_hiddenProperties.insert(key);
            m_prop_map.value(key)->hide();
        }

        m_icons.clear();

        Q_FOREACH (const Property &prop, m_props) {
            if (! m_hiddenProperties.contains(prop.key)) {
                m_icons << m_prop_map.value(prop.key);
                m_prop_map.value(prop.key)->show();
            } else {
                m_prop_map.value(prop.key)->hide();
            }
        }

        if (m_enableCollapse) {
            if (m_hiddenProperties.contains("__collapse")) {
                m_collapseIcon->hide();
            } else {
                m_icons << m_collapseIcon;
                m_collapseIcon->show();
            }
        } else {
            m_collapseIcon->hide();
        }

        m_layout->setItems(m_icons);

        emit iconCountChanged();

    } else {
        kWarning() << "qobject_cast failed";
    }

}

#include "kimstatusbargraphics.moc"
// vim: sw=4 sts=4 et tw=100
