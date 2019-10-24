/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2009 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "FontInstInterface.h"
#include "FontinstIface.h"
#include "FontInst.h"
#include <kio/global.h>
#include <QDebug>
#include <QProcess>
#include <QDBusServiceWatcher>
#include "config-fontinst.h"
#include "debug.h"

#define KFI_DBUG qCDebug(KCM_KFONTINST_KIO) << '(' << time(NULL) << ')'

namespace KFI
{

FontInstInterface::FontInstInterface()
                 : itsInterface(new OrgKdeFontinstInterface(OrgKdeFontinstInterface::staticInterfaceName(),
                                                            FONTINST_PATH,
                                                            QDBusConnection::sessionBus(), nullptr))
                 , itsActive(false)
{
    KFI_DBUG;
    FontInst::registerTypes();
    
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(QLatin1String(OrgKdeFontinstInterface::staticInterfaceName()),
                                                           QDBusConnection::sessionBus(),
                                                           QDBusServiceWatcher::WatchForOwnerChange, this);

    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &FontInstInterface::dbusServiceOwnerChanged);
    connect(itsInterface, &OrgKdeFontinstInterface::status, this, &FontInstInterface::status);
    connect(itsInterface, &OrgKdeFontinstInterface::fontList, this, &FontInstInterface::fontList);
    connect(itsInterface, &OrgKdeFontinstInterface::fontStat, this, &FontInstInterface::fontStat);
    
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(OrgKdeFontinstInterface::staticInterfaceName()))
        QProcess::startDetached(QLatin1String(KFONTINST_LIB_EXEC_DIR"/fontinst"), QStringList());
}

FontInstInterface::~FontInstInterface()
{
    KFI_DBUG;
}

int FontInstInterface::install(const QString &file, bool toSystem)
{
    KFI_DBUG;
    itsInterface->install(file, true, toSystem, getpid(), true);
    return waitForResponse();
}

int FontInstInterface::uninstall(const QString &name, bool fromSystem)
{
    KFI_DBUG;
    itsInterface->uninstall(name, fromSystem, getpid(), true);
    return waitForResponse();
}

int FontInstInterface::reconfigure()
{
    KFI_DBUG;
    itsInterface->reconfigure(getpid(), false);
    return waitForResponse();
}

Families FontInstInterface::list(bool system)
{
    KFI_DBUG;
    Families rv;
    itsInterface->list(system ? FontInst::SYS_MASK : FontInst::USR_MASK, getpid());
    if(FontInst::STATUS_OK==waitForResponse())
    {
        rv=itsFamilies;
        itsFamilies=Families();
    }
    return rv;
}

Family FontInstInterface::statFont(const QString &file, bool system)
{
    KFI_DBUG;
    Family rv;
    itsInterface->statFont(file, system ? FontInst::SYS_MASK : FontInst::USR_MASK, getpid());
    if(FontInst::STATUS_OK==waitForResponse())
    {
        rv=*itsFamilies.items.begin();
        itsFamilies=Families();
    }
    return rv;
}

QString FontInstInterface::folderName(bool sys)
{
    if(!itsInterface)
        return QString();

    QDBusPendingReply<QString> reply=itsInterface->folderName(sys);

    reply.waitForFinished();
    return reply.isError() ? QString() : reply.argumentAt<0>();
}

int FontInstInterface::waitForResponse()
{
    KFI_DBUG;
    itsStatus=FontInst::STATUS_OK;
    itsFamilies=Families();
    itsActive=true;

    itsEventLoop.exec();
    KFI_DBUG << "Loop finished";
    return itsStatus;
}

void FontInstInterface::dbusServiceOwnerChanged(const QString &name, const QString &from, const QString &to)
{
    if(itsActive && to.isEmpty() && !from.isEmpty() && name==QLatin1String(OrgKdeFontinstInterface::staticInterfaceName()))
    {
        KFI_DBUG << "Service died :-(";
        itsStatus=FontInst::STATUS_SERVICE_DIED;
        itsEventLoop.quit();
    }
}

void FontInstInterface::status(int pid, int value)
{
    if(itsActive && pid==getpid())
    {
        KFI_DBUG << "Status:" << value;
        itsStatus=value;
        itsEventLoop.quit();
    }
}

void FontInstInterface::fontList(int pid, const QList<KFI::Families> &families)
{
    if(itsActive && pid==getpid())
    {
        KFI_DBUG;
        itsFamilies=1==families.count() ? *families.begin() : Families();
        itsStatus=1==families.count() ? (int)FontInst::STATUS_OK : (int)KIO::ERR_DOES_NOT_EXIST;
        itsEventLoop.quit();
    }
}

void FontInstInterface::fontStat(int pid, const KFI::Family &font)
{
    if(itsActive && pid==getpid())
    {
        KFI_DBUG;
        itsFamilies=Families(font, false);
        itsStatus=font.styles().count()>0 ? (int)FontInst::STATUS_OK : (int)KIO::ERR_DOES_NOT_EXIST;
        itsEventLoop.quit();
    }
}

}
