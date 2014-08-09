/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "kickoffplugin.h"
#include "applicationmodel.h"
#include "favoritesmodel.h"
#include "krunnermodel.h"
#include "leavemodel.h"
#include "recentlyusedmodel.h"
#include "systemmodel.h"
#include "urlitemlauncher.h"
#include "processrunner.h"

#include <QtQml>

void KickoffPlugin::registerTypes (const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.private.kickoff"));
    qmlRegisterType<Kickoff::ApplicationModel>(uri, 0, 1, "ApplicationModel");
    qmlRegisterType<Kickoff::FavoritesModel>(uri, 0, 1, "FavoritesModel");
    qmlRegisterType<Kickoff::KRunnerModel>(uri, 0, 1, "KRunnerModel");
    qmlRegisterType<Kickoff::LeaveModel>(uri, 0, 1, "LeaveModel");
    qmlRegisterType<Kickoff::RecentlyUsedModel>(uri, 0, 1, "RecentlyUsedModel");
    qmlRegisterType<Kickoff::SystemModel>(uri, 0, 1, "SystemModel");
    qmlRegisterType<Kickoff::UrlItemLauncher>(uri, 0, 1, "Launcher");
    qmlRegisterType<Kickoff::ProcessRunner>(uri, 0, 1, "ProcessRunner");
}

#include "kickoffplugin.moc"
