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

#include "paneldialog.h"
#include "kimstatusbargraphics.h"
#include "kimstatusbar.h"
#include "kimlookuptable.h"
#include "kimpanelsettings.h"
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
#include <kcmultidialog.h>
#include <kcmodule.h>
#include <kservicetypetrader.h>
#include <plasma/corona.h>
#include <plasma/theme.h>
#include <plasma/widgets/iconwidget.h>

KIMPanel::KIMPanel(QObject* parent)
    : QObject(parent),
    m_panel_agent(0),
    m_statusbarGraphics(0)
{
    m_panel_agent = new PanelAgent(this);

    m_statusbarGraphics = new KIMStatusBarGraphics(m_panel_agent);

    m_statusbar = new KIMStatusBar();

    m_statusbar->addAction(KStandardAction::quit(qApp,SLOT(quit()),this));

    m_statusbar->setGraphicsWidget(m_statusbarGraphics);

    m_lookup_table = new KIMLookupTable(m_panel_agent);

    m_statusbar->show();

    m_statusbar->move(KIM::Settings::self()->floatingStatusbarPos());



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

    m_panel_agent->created();
}

KIMPanel::~KIMPanel()
{
    delete m_statusbar;
//    delete m_statusbarGraphics;
    delete m_lookup_table;
}

void KIMPanel::about()
{
    KAboutApplicationDialog dlg(KCmdLineArgs::aboutData());
    dlg.exec();
}

void KIMPanel::exit()
{
//X     m_panel_agent->exit();
    delete m_statusbar;
    delete m_statusbarGraphics;
    delete m_lookup_table;
    KApplication::kApplication()->quit();
}

void KIMPanel::showConfig()
{
#if 0
    QString constraint = "[X-KDE-System-Settings-Parent-Category] == 'input-method'";
    KService::List modules = KServiceTypeTrader::self()->query("KCModule",constraint);

    KCMultiDialog *dialog = new KCMultiDialog();

    foreach (const KService::Ptr &srv, modules) {
        kDebug() << srv->library();
        dialog->addModule(srv->library());
    }

    dialog->show();

    delete dialog;
#endif
}

#include "paneldialog.moc"
