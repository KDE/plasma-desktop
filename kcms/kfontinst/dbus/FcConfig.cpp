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

#include "FcConfig.h"
#include "Misc.h"
#include <QRegExp>
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomText>
#include <QDebug>
#include <stdio.h>
#include <fontconfig/fontconfig.h>

#define KFI_DBUG qDebug() << time(nullptr)

namespace KFI
{

namespace FcConfig
{

inline QString xDirSyntax(const QString &d) { return Misc::fileSyntax(d); }

//
// Obtain location of config file to use.
//
// For system, prefer the following:
//
//     <...>/config.d/00kde.conf   = preferred method from FCConfig >= 2.3
//     <...>/local.conf
//
// Non-system, prefer:
//
//     $HOME/<...>/.fonts.conf
//     $HOME/<...>/fonts.conf
//
QString getConfigFile(bool system)
{
#if (FC_VERSION>=20300)
    static const char constKdeRootFcFile[] = "00kde.conf";
#endif

    FcStrList   *list=FcConfigGetConfigFiles(FcConfigGetCurrent());
    QStringList files;
    FcChar8     *file;
    QString     home(Misc::dirSyntax(QDir::homePath()));

    while((file=FcStrListNext(list)))
    {
        QString f((const char *)file);

        if(Misc::fExists(f))
        {
            // For nonsystem, only consider file within $HOME
            if(system || 0==Misc::fileSyntax(f).indexOf(home))
                files.append(f);
        }
#if (FC_VERSION>=20300)
        if(system && Misc::dExists(f) && (f.contains(QRegExp("/conf\\.d/?$")) ||
                                          f.contains(QRegExp("/conf\\.d?$"))) )
            return Misc::dirSyntax(f)+constKdeRootFcFile;   // This ones good enough for me!
#endif
    }

    //
    // Go through list of files, looking for the preferred one...
    if(!files.isEmpty())
    {
        QStringList::const_iterator it(files.begin()),
                              end(files.end());

        for(; it!=end; ++it)
            if(-1!=(*it).indexOf(QRegExp(system ? "/local\\.conf$" : "/\\.?fonts\\.conf$")))
                return *it;
        return files.front();  // Just return the 1st one...
    }
    else // Hmmm... no known files?
        return system ? "/etc/fonts/local.conf" : Misc::fileSyntax(home+"/.fonts.conf");
}

void addDir(const QString &dir, bool system)
{
    QDomDocument doc("fontconfig");
    QString      fileName=getConfigFile(system);
    QFile        f(fileName);
    bool         hasDir(false);

    KFI_DBUG << "Using fontconfig file:" << fileName;

    // Load existing file - and check to see whether it has the dir...
    if(f.open(QIODevice::ReadOnly))
    {
        doc.clear();

        if(doc.setContent(&f))
        {
            QDomNode n = doc.documentElement().firstChild();

            while(!n.isNull() && !hasDir)
            {
                QDomElement e = n.toElement();

                if(!e.isNull() && "dir"==e.tagName())
                    if(0==Misc::expandHome(Misc::dirSyntax(e.text())).indexOf(dir))
                        hasDir=true;
                n=n.nextSibling();
            }
        }
        f.close();
    }

    // Add dir, and save, if config does not already have this dir.
    if(!hasDir)
    {
        if(doc.documentElement().isNull())
            doc.appendChild(doc.createElement("fontconfig"));

        QDomElement newNode = doc.createElement("dir");
        QDomText    text    = doc.createTextNode(Misc::contractHome(xDirSyntax(dir)));

        newNode.appendChild(text);
        doc.documentElement().appendChild(newNode);

        FcAtomic *atomic=FcAtomicCreate((const unsigned char *)(QFile::encodeName(fileName).data()));

        if(atomic)
        {
            if(FcAtomicLock(atomic))
            {
                FILE *f=fopen((char *)FcAtomicNewFile(atomic), "w");

                if(f)
                {
                    //
                    // Check document syntax...
                    static const char qtXmlHeader[]   = "<?xml version = '1.0'?>";
                    static const char xmlHeader[]     = "<?xml version=\"1.0\"?>";
                    static const char qtDocTypeLine[] = "<!DOCTYPE fontconfig>";
                    static const char docTypeLine[]   = "<!DOCTYPE fontconfig SYSTEM "
                                                        "\"fonts.dtd\">";

                    QString str(doc.toString());
                    int     idx;

                    if(0!=str.indexOf("<?xml"))
                        str.insert(0, xmlHeader);
                    else if(0==str.indexOf(qtXmlHeader))
                        str.replace(0, strlen(qtXmlHeader), xmlHeader);

                    if(-1!=(idx=str.indexOf(qtDocTypeLine)))
                        str.replace(idx, strlen(qtDocTypeLine), docTypeLine);

                    //
                    // Write to file...
                    fputs(str.toUtf8(), f);
                    fclose(f);

                    if(!FcAtomicReplaceOrig(atomic))
                        FcAtomicDeleteNew(atomic);
                }
                FcAtomicUnlock(atomic);
            }
            FcAtomicDestroy(atomic);
        }
    }
}

}

}
