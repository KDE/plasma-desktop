/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
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

#include "kickerplugin.h"
#include "abstractmodel.h"
#include "appsmodel.h"
#include "computermodel.h"
#include "containmentinterface.h"
#include "draghelper.h"
#include "simplefavoritesmodel.h"
#include "kastatsfavoritesmodel.h"
#include "dashboardwindow.h"
#include "funnelmodel.h"
#include "processrunner.h"
#include "recentusagemodel.h"
#include "rootmodel.h"
#include "runnermodel.h"
#include "submenu.h"
#include "systemmodel.h"
#include "systemsettings.h"
#include "wheelinterceptor.h"
#include "windowsystem.h"

#include <QQmlEngine>

void KickerPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.kicker"));

    qmlRegisterType<AbstractModel>();

    qmlRegisterType<AppsModel>(uri, 0, 1, "AppsModel");
    qmlRegisterType<ComputerModel>(uri, 0, 1, "ComputerModel");
    qmlRegisterType<ContainmentInterface>(uri, 0, 1, "ContainmentInterface");
    qmlRegisterType<DragHelper>(uri, 0, 1, "DragHelper");
    qmlRegisterType<SimpleFavoritesModel>(uri, 0, 1, "FavoritesModel");
    qmlRegisterType<KAStatsFavoritesModel>(uri, 0, 1, "KAStatsFavoritesModel");
    qmlRegisterType<DashboardWindow>(uri, 0, 1, "DashboardWindow");
    qmlRegisterType<FunnelModel>(uri, 0, 1, "FunnelModel");
    qmlRegisterType<ProcessRunner>(uri, 0, 1, "ProcessRunner");
    qmlRegisterType<RecentUsageModel>(uri, 0, 1, "RecentUsageModel");
    qmlRegisterType<RootModel>(uri, 0, 1, "RootModel");
    qmlRegisterType<RunnerModel>(uri, 0, 1, "RunnerModel");
    qmlRegisterType<SubMenu>(uri, 0, 1, "SubMenu");
    qmlRegisterType<SystemModel>(uri, 0, 1, "SystemModel");
    qmlRegisterType<SystemSettings>(uri, 0, 1, "SystemSettings");
    qmlRegisterType<WheelInterceptor>(uri, 0, 1, "WheelInterceptor");
    qmlRegisterType<WindowSystem>(uri, 0, 1, "WindowSystem");
}
