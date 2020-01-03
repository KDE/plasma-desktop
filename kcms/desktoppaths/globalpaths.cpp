/**
 *  Copyright (c) Martin R. Jones 1996
 *  Copyright (c) Bernd Wuebben 1998
 *  Copyright (c) Christian Tibirna 1998
 *  Copyright 1998-2007 David Faure <faure@kde.org>
 *  Copyright (c) 2010 Matthias Fuchs <mat69@gmx.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

//
//
// "Desktop Options" Tab for KDesktop configuration
//
// (c) Martin R. Jones 1996
// (c) Bernd Wuebben 1998
//
// Layouts
// (c) Christian Tibirna 1998
// Port to KControl, split from Misc Tab, Port to KControl2
// (c) David Faure 1998
// Desktop menus, paths
// (c) David Faure 2000


// Own
#include "globalpaths.h"
#include "ui_globalpaths.h"
#include "desktoppathssettings.h"

#include <KPluginFactory>

K_PLUGIN_FACTORY(KcmDesktopPathsFactory, registerPlugin<DesktopPathConfig>();)

//-----------------------------------------------------------------------------

DesktopPathConfig::DesktopPathConfig(QWidget *parent, const QVariantList &)
    : KCModule( parent )
    , m_ui(new Ui::DesktopPathsView)
    , m_pathsSettings(new DesktopPathsSettings(this))
{
    m_ui->setupUi(this);
    setQuickHelp(i18n("<h1>Paths</h1>\n"
                      "This module allows you to choose where in the filesystem the "
                      "files on your desktop should be stored.\n"
                      "Use the \"Whats This?\" (Shift+F1) to get help on specific options."));
    addConfig(m_pathsSettings, this);
}

DesktopPathConfig::~DesktopPathConfig()
{
}

#include "globalpaths.moc"
