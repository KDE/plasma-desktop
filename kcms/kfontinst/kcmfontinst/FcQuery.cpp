/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
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

#include "FcQuery.h"
#include "Fc.h"
#include <QtCore/QProcess>
#include <stdio.h>

namespace KFI
{

// key: 0(i)(s)
static int getInt(const QString &str)
{
    int rv=KFI_NULL_SETTING,
        start=str.lastIndexOf(':')+1,
        end=str.lastIndexOf("(i)(s)");

    if(end>start)
        rv=str.mid(start, end-start).trimmed().toInt();

    return rv;
}

CFcQuery::~CFcQuery()
{
}

void CFcQuery::run(const QString &query)
{
    QStringList args;

    itsFile=itsFont=QString();
    itsBuffer=QByteArray();

    if(itsProc)
        itsProc->kill();
    else
        itsProc=new QProcess(this);

    args << "-v" << query;

    connect(itsProc, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(procExited()));
    connect(itsProc, SIGNAL(readyReadStandardOutput()), SLOT(data()));

    itsProc->start("fc-match", args);
}

void CFcQuery::procExited()
{
    QString     family;
    int         weight(KFI_NULL_SETTING), slant(KFI_NULL_SETTING), width(KFI_NULL_SETTING);
    QStringList results(QString::fromUtf8(itsBuffer, itsBuffer.length()).split('\n'));

    if(results.size())
    {
        QStringList::ConstIterator it(results.begin()),
                                   end(results.end());

        for(; it!=end; ++it)
        {
            QString line((*it).trimmed());

            if(0==line.indexOf("file:"))  // file: "Wibble"(s)
            {
                int endPos=line.indexOf("\"(s)");

                if(-1!=endPos)
                    itsFile=line.mid(7, endPos-7);
            }
            else if(0==line.indexOf("family:"))  // family: "Wibble"(s)
            {
                int endPos=line.indexOf("\"(s)");

                if(-1!=endPos)
                    family=line.mid(9, endPos-9);
            }
            else if(0==line.indexOf("slant:"))  // slant: 0(i)(s)
                slant=getInt(line);
            else if(0==line.indexOf("weight:"))  // weight: 0(i)(s)
                weight=getInt(line);
            else if(0==line.indexOf("width:"))  // width: 0(i)(s)
                width=getInt(line);
        }
    }

    if(!family.isEmpty())
        itsFont=FC::createName(family, weight, width, slant);

    emit finished();
}

void CFcQuery::data()
{
    itsBuffer+=itsProc->readAllStandardOutput();
}

}

#include "FcQuery.moc"
