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

#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>
#include <QProcess>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QStandardPaths>
#include <QDebug>
#include <QSaveFile>
#include <KShell>
#include <fontconfig/fontconfig.h>
#include "Folder.h"
#include "FcConfig.h"
#include "Misc.h"
#include "KfiConstants.h"
#include "XmlStrings.h"
#include "Style.h"
#include "File.h"
#include "config-fontinst.h"

#define DISABLED_FONTS "disabledfonts"
#define KFI_DBUG qDebug() << time(nullptr)

namespace KFI
{

bool Folder::CfgFile::modified()
{
    return timestamp!=Misc::getTimeStamp(name);
}

void Folder::CfgFile::updateTimeStamp()
{
    timestamp=Misc::getTimeStamp(name);
}

Folder::~Folder()
{
    saveDisabled();
}

void Folder::init(bool system, bool systemBus)
{
    itsIsSystem=system;
    if(!system)
    {
        FcStrList   *list=FcConfigGetFontDirs(FcInitLoadConfigAndFonts());
        QStringList dirs;
        FcChar8     *fcDir;

        while((fcDir=FcStrListNext(list)))
            dirs.append(Misc::dirSyntax((const char *)fcDir));
    
        itsLocation=Misc::getFolder(Misc::dirSyntax(QDir::homePath()+"/.fonts/"), Misc::dirSyntax(QDir::homePath()), dirs);
    }
    else
        itsLocation=KFI_DEFAULT_SYS_FONTS_FOLDER;

    if((!system && !systemBus) || (system && systemBus))
        FcConfig::addDir(itsLocation, system);

    itsDisabledCfg.dirty=false;
    if(itsDisabledCfg.name.isEmpty())
    {
        QString fileName("/" DISABLED_FONTS ".xml");

        if(system)
            itsDisabledCfg.name=QString::fromLatin1(KFI_ROOT_CFG_DIR)+fileName;
        else
        {
            QString path=QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/');

            if(!Misc::dExists(path))
                Misc::createDir(path);
            itsDisabledCfg.name=path+fileName;
        }
        itsDisabledCfg.timestamp=0;
    }
}

bool Folder::allowToggling() const
{
    return Misc::fExists(itsDisabledCfg.name)
            ? Misc::fWritable(itsDisabledCfg.name)
            : Misc::dWritable(Misc::getDir(itsDisabledCfg.name));
}

void Folder::loadDisabled()
{
    if(itsDisabledCfg.dirty)
        saveDisabled();

    QFile f(itsDisabledCfg.name);

    KFI_DBUG << itsDisabledCfg.name;
    itsDisabledCfg.dirty=false;
    if(f.open(QIODevice::ReadOnly))
    {
        QDomDocument doc;

        if(doc.setContent(&f))
            for(QDomNode n=doc.documentElement().firstChild(); !n.isNull(); n=n.nextSibling())
            {
                QDomElement e=n.toElement();

                if(FONT_TAG==e.tagName())
                {
                    Family fam(e, false);

                    if(!fam.name().isEmpty())
                    {
                        Style style(e, false);

                        if(KFI_NO_STYLE_INFO!=style.value())
                        {
                            QList<File> files;

                            if(e.hasAttribute(PATH_ATTR))
                            {
                                File file(e, true);

                                if(!file.path().isEmpty())
                                    files.append(file);
                                else
                                    itsDisabledCfg.dirty=true;
                            }
                            else
                            {
                                for(QDomNode n=e.firstChild(); !n.isNull(); n=n.nextSibling())
                                {
                                    QDomElement ent=n.toElement();

                                    if(FILE_TAG==ent.tagName())
                                    {
                                        File file(ent, true);

                                        if(!file.path().isEmpty())
                                            files.append(file);
                                        else
                                        {
                                            KFI_DBUG << "Set dirty from load";
                                            itsDisabledCfg.dirty=true;
                                        }
                                    }
                                }
                            }

                            if(files.count()>0)
                            {
                                QList<File>::ConstIterator it(files.begin()),
                                                           end(files.end());

                                FamilyCont::ConstIterator f(itsFonts.insert(fam));
                                StyleCont::ConstIterator  s((*f).add(style));

                                for(; it!=end; ++it)
                                    (*s).add(*it);
                            }
                        }
                    }
                }
            }

        f.close();
        itsDisabledCfg.updateTimeStamp();
    }

    saveDisabled();
}

void Folder::saveDisabled()
{
    if(itsDisabledCfg.dirty)
    {
        if(!itsIsSystem || Misc::root())
        {
            KFI_DBUG << itsDisabledCfg.name;

            QSaveFile file;

            file.setFileName(itsDisabledCfg.name);

            if (!file.open(QIODevice::WriteOnly))
            {
                KFI_DBUG << "Exit - cant open save file";
                qApp->exit(0);
            }

            QTextStream str(&file);

            str << "<" DISABLED_FONTS ">" << Qt::endl;

            FamilyCont::ConstIterator it(itsFonts.begin()),
                                      end(itsFonts.end());

            for(; it!=end; ++it)
                (*it).toXml(true, str);
            str << "</" DISABLED_FONTS ">" << Qt::endl;
            str.flush();

            if(!file.commit())
            {
                KFI_DBUG << "Exit - cant finalize save file";
                qApp->exit(0);
            }
        }
        itsDisabledCfg.updateTimeStamp();
        itsDisabledCfg.dirty=false;
    }
}

QStringList Folder::toXml(int max)
{
    QStringList               rv;
    FamilyCont::ConstIterator it(itsFonts.begin()),
                              end(itsFonts.end());
    QString                   string;
    QTextStream               str(&string);

    for(int i=0; it!=end; ++it, ++i)
    {
        if(0==(i%max))
        {
            if(i)
            {
                str << "</" FONTLIST_TAG ">" << Qt::endl;
                rv.append(string);
                string=QString();
            }
            str << "<" FONTLIST_TAG " " << SYSTEM_ATTR "=\"" << (itsIsSystem ? "true" : "false") << "\">" << Qt::endl;
        }

        (*it).toXml(false, str);
    }

    if(!string.isEmpty())
    {
        str << "</" FONTLIST_TAG ">" << Qt::endl;
        rv.append(string);
    }
    return rv;
}

Families Folder::list()
{
    Families                  fam(itsIsSystem);
    FamilyCont::ConstIterator it(itsFonts.begin()),
                              end(itsFonts.end());

    for(int i=0; it!=end; ++it, ++i)
        fam.items.insert(*it);

    return fam;
}

bool Folder::contains(const QString &family, quint32 style)
{
    FamilyCont::ConstIterator fam=itsFonts.find(Family(family));

    if(fam==itsFonts.end())
        return false;

    StyleCont::ConstIterator st=(*fam).styles().find(Style(style));

    return st!=(*fam).styles().end();
}

void Folder::add(const Family &family)
{
    FamilyCont::ConstIterator existingFamily=itsFonts.find(family);

    if(existingFamily==itsFonts.end())
        itsFonts.insert(family);
    else
    {
        StyleCont::ConstIterator it(family.styles().begin()),
                                 end(family.styles().end());

        for(; it!=end; ++it)
        {
            StyleCont::ConstIterator existingStyle=(*existingFamily).styles().find(*it);

            if(existingStyle==(*existingFamily).styles().end())
                (*existingFamily).add(*it);
            else
            {
                FileCont::ConstIterator fit((*it).files().begin()),
                                        fend((*it).files().end());

                for(; fit!=fend; ++fit)
                {
                    FileCont::ConstIterator f=(*existingStyle).files().find(*fit);

                    if(f==(*existingStyle).files().end())
                        (*existingStyle).add(*fit);
                }

                (*existingStyle).setWritingSystems((*existingStyle).writingSystems()|(*it).writingSystems());
                if(!(*existingStyle).scalable() && (*it).scalable())
                    (*existingStyle).setScalable(true);
            }
        }
    }
}
    
void Folder::configure(bool force)
{
KFI_DBUG << "EMPTY MODIFIED " << itsModifiedDirs.isEmpty();

    if(force || !itsModifiedDirs.isEmpty())
    {
        saveDisabled();

        QSet<QString>::ConstIterator it(itsModifiedDirs.constBegin()),
                                     end(itsModifiedDirs.constEnd());
        QSet<QString>                dirs;

        for(; it!=end; ++it)
            if(Misc::fExists((*it)+"fonts.dir"))
                dirs.insert(KShell::quoteArg(*it));

        if(!dirs.isEmpty())
            QProcess::startDetached(QStringLiteral(KFONTINST_LIB_EXEC_DIR"/fontinst_x11"), dirs.values());

        itsModifiedDirs.clear();

KFI_DBUG << "RUN FC";
        Misc::doCmd("fc-cache");
KFI_DBUG << "DONE";
    }
}

Folder::Flat Folder::flatten() const
{
    FamilyCont::ConstIterator fam=itsFonts.begin(),
                              famEnd=itsFonts.end();
    Flat                      rv;

    for(; fam!=famEnd; ++fam)
    {
        StyleCont::ConstIterator style((*fam).styles().begin()),
                                 styleEnd((*fam).styles().end());

        for(; style!=styleEnd; ++style)
        {
            FileCont::ConstIterator file((*style).files().begin()),
                                    fileEnd((*style).files().end());

            for(; file!=fileEnd; ++file)
                rv.insert(FlatFont(*fam, *style, *file));
        }
    }

    return rv;
}

Families Folder::Flat::build(bool system) const
{
    ConstIterator it(begin()),
                  e(end());
    Families      families(system);

    for(; it!=e; ++it)
    {
        Family                    f((*it).family);
        Style                     s((*it).styleInfo, (*it).scalable, (*it).writingSystems);
        FamilyCont::ConstIterator fam=families.items.constFind(f);

        if(families.items.constEnd()==fam)
        {
            s.add((*it).file);
            f.add(s);
            families.items.insert(f);
        }
        else
        {
            StyleCont::ConstIterator st=(*fam).styles().constFind(s);

            if((*fam).styles().constEnd()==st)
            {
                s.add((*it).file);
                (*fam).add(s);
            }
            else
                (*st).add((*it).file);
        }
    }

    return families;
}

}
