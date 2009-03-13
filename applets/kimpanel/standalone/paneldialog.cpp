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

#include "paneldialog.h"
#include "kimpanelagent.h"
#include "kimstatusbargraphics.h"
#include "kimstatusbar.h"
#include "kimlookuptable.h"
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kaboutapplicationdialog.h>
#include <kapplication.h>
#include <kaction.h>
#include <kstandardaction.h>
#include <kmenu.h>
#include <kicon.h>
#include <kmessagebox.h>
#include <kwindowsystem.h>
#include <plasma/corona.h>
#include <plasma/theme.h>
#include <plasma/widgets/iconwidget.h>
#include <QtCore>
#include <QtGui>

KIMPanel::KIMPanel(QObject* parent)
    : QObject(parent),
    m_panel_agent(0)
{
    m_panel_agent = new PanelAgent(this);

    m_statusbarGraphics = new KIMStatusBarGraphics(m_panel_agent);
//X     m_statusbarGraphics->showLogo(true);

    m_statusbar = new KIMStatusBar();
    m_statusbar->addAction(KStandardAction::quit(qApp,SLOT(quit()),this));

    m_statusbar->setGraphicsWidget(m_statusbarGraphics);

    m_lookup_table = new KIMLookupTable(m_panel_agent);

//X     connect(m_panel_agent,
//X         SIGNAL(enable(bool)),
//X         SLOT(enable(bool)));
    m_statusbar->show();


#if 0
    // create actions
    KAction *action = new KAction(KIcon("view-refresh"),i18n("Reload Config"),this);
    connect(action,SIGNAL(triggered()),m_panel_agent,SIGNAL(ReloadConfig()));
    m_actions << action;
    m_actions << KStandardAction::aboutApp(this,SLOT(about()),this);
    m_actions << KStandardAction::quit(this,SLOT(exit()),this);

    m_statusbar->addActions(m_actions);
    m_statusbar->setContextMenuPolicy(Qt::ActionsContextMenu);
#endif

}

KIMPanel::~KIMPanel()
{
    //delete m_statusbar;
    //delete m_lookup_table;
}

// ------------------ handle panel agent signal start-----------------

#if 0
void KIMPanel::showPreedit(bool to_show)
{
    m_lookup_table->showPreedit(to_show);
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
#endif

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
