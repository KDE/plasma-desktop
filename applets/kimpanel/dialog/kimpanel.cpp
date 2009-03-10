/*
 *   Copyright (C) 2009 Wang Hoi <zealot.hoigmail.com>
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

#include "kimpanel.h"
#include "kimpanelagent.h"
#include "statusbarwidget.h"
#include "lookuptablewidget.h"
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kaboutapplicationdialog.h>
#include <kapplication.h>
#include <kaction.h>
#include <kstandardaction.h>
#include <kmenu.h>
#include <kicon.h>
#include <kmessagebox.h>
#include <plasma/theme.h>
#include <QtCore>
#include <QtGui>

KIMPanel::KIMPanel(QObject* parent)
    : QObject(parent),
    m_panel_agent(0)
{
    m_panel_agent = new PanelAgent(this);


//X     connect(m_panel_agent,
//X         SIGNAL(enable(bool)),
//X         SLOT(enable(bool)));
    connect(m_panel_agent,
        SIGNAL(showPreedit(bool)),
        SLOT(showPreedit(bool)));
    connect(m_panel_agent,
        SIGNAL(showAux(bool)),
        SLOT(showAux(bool)));
    connect(m_panel_agent,
        SIGNAL(showLookupTable(bool)),
        SLOT(showLookupTable(bool)));

    connect(m_panel_agent,
        SIGNAL(execDialog(const Property &)),
        SLOT(execDialog(const Property &)));
    connect(m_panel_agent,
        SIGNAL(execMenu(const QList<Property> &)),
        SLOT(execMenu(const QList<Property> &)));


    m_statusbar = new StatusBarWidget();

    connect(m_panel_agent,
        SIGNAL(registerProperties(const QList<Property> &)),
        m_statusbar,
        SLOT(registerProperties(const QList<Property> &)));
    connect(m_panel_agent,
        SIGNAL(updateProperty(const Property &)),
        m_statusbar,
        SLOT(updateProperty(const Property &)));
    connect(m_statusbar,SIGNAL(triggerProperty(const QString &)),m_panel_agent,SIGNAL(TriggerProperty(const QString &)));

    m_statusbar->show();


    m_lookup_table = new LookupTableWidget();
    connect(m_panel_agent,
        SIGNAL(updateSpotLocation(int,int)),
        m_lookup_table,
        SLOT(updateSpotLocation(int,int)));
    connect(m_panel_agent,
        SIGNAL(updateLookupTable(const LookupTable &)),
        m_lookup_table,
        SLOT(updateLookupTable(const LookupTable &)));
    connect(m_panel_agent,
        SIGNAL(updateAux(const QString &,const QList<TextAttribute> &)),
        m_lookup_table,
        SLOT(updateAux(const QString &,const QList<TextAttribute> &)));

    // create actions
    KAction *action = new KAction(KIcon("view-refresh"),i18n("Reload Config"),this);
    connect(action,SIGNAL(triggered()),m_panel_agent,SIGNAL(ReloadConfig()));
    m_actions << action;
    m_actions << KStandardAction::aboutApp(this,SLOT(about()),this);
    m_actions << KStandardAction::quit(this,SLOT(exit()),this);

    m_statusbar->addActions(m_actions);
    m_statusbar->setContextMenuPolicy(Qt::ActionsContextMenu);

    m_panel_agent->created();
}

KIMPanel::~KIMPanel()
{
    delete m_statusbar;
    delete m_lookup_table;
}

// ------------------ handle panel agent signal start-----------------

void KIMPanel::execDialog(const Property &prop) {
    KMessageBox::information(NULL,prop.tip,prop.label);
}

void KIMPanel::execMenu(const QList<Property> &prop_list)
{
    QMenu *menu = new QMenu();
    QSignalMapper *mapper = new QSignalMapper(this);
    connect(mapper,SIGNAL(mapped(const QString&)),m_panel_agent,SIGNAL(TriggerProperty(const QString &)));
    foreach (const Property &prop, prop_list) {
        QAction *action = new QAction(QIcon(prop.icon),prop.label,this);
        mapper->setMapping(action,prop.key);
        connect(action,SIGNAL(triggered()),mapper,SLOT(map()));
        menu->addAction(action);
    }
    menu->exec(QCursor::pos());
    delete menu;
}

void KIMPanel::showPreedit(bool to_show)
{
}

void KIMPanel::showAux(bool to_show)
{
    m_lookup_table->showAux(to_show);
}

void KIMPanel::showLookupTable(bool to_show)
{
    m_lookup_table->showLookupTable(to_show);
    //m_lookup_table->setVisible(to_show);
}

void KIMPanel::about()
{
    KAboutApplicationDialog dlg(KCmdLineArgs::aboutData());
    dlg.exec();
}

void KIMPanel::exit()
{
//X     m_panel_agent->exit();
    KApplication::kApplication()->quit();
}

// ------------------ handle panel agent signal end -----------------

// ------------------ handle ui signal start-----------------

// ------------------ handle ui signal end -------------------

// ------------------ internal function start -------------------

void KIMPanel::showConfig()
{
    KMessageBox::information(NULL,"Implement me!","Config");
}

// ------------------ internal function start -------------------
#include "kimpanel.moc"
