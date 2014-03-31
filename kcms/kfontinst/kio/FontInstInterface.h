#ifndef __FONTINST_INTERFACE_H__
#define __FONTINST_INTERFACE_H__

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

#include <QtCore/QObject>
#include <QtCore/QEventLoop>
#include "Family.h"

class QTimer;
class OrgKdeFontinstInterface;

namespace KFI
{

class FontInstInterface : public QObject
{
    Q_OBJECT

    public:

    FontInstInterface();
    virtual ~FontInstInterface();

    int      install(const QString &file, bool toSystem);
    int      uninstall(const QString &name, bool fromSystem);
    int      reconfigure();
    Families list(bool system);
    Family   stat(const QString &file, bool system);
    QString  folderName(bool sys);

    private:

    int waitForResponse();

    private Q_SLOTS:

    void dbusServiceOwnerChanged(const QString &name, const QString &from, const QString &to);
    void status(int pid, int value);
    void fontList(int pid, const QList<KFI::Families> &families);
    void fontStat(int pid, const KFI::Family &font);

    private:

    OrgKdeFontinstInterface *itsInterface;
    bool                    itsActive;
    int                     itsStatus;
    Families                itsFamilies;
    QEventLoop              itsEventLoop;
};

}

#endif
