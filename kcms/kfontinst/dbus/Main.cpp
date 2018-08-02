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

#include "FontInst.h"
#include "Misc.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    // KJob uses a QEventLoopLocker which causes kfontinst to quit
    // after the job is done, prevent this by disabling quit lock.
    QCoreApplication::setQuitLockEnabled(false);

    QCoreApplication *app=new QCoreApplication(argc, argv);
    KFI::FontInst    fi;

    int rv=app->exec();
    delete app;
    return rv;
}
