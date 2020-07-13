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

#include <QDBusConnection>
#include <QTimer>
#include <QDebug>
#include <KAuth>
#include <kio/global.h>
#include <fontconfig/fontconfig.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "FontInst.h"
#include "fontinstadaptor.h"
#include "Misc.h"
#include "Fc.h"
#include "WritingSystems.h"
#include "Utils.h"
#include "FontinstIface.h"

#define KFI_DBUG qDebug() << time(nullptr)

namespace KFI
{

static void decompose(const QString &name, QString &family, QString &style)
{
    int commaPos=name.lastIndexOf(',');

    family=-1==commaPos ? name : name.left(commaPos);
    style=-1==commaPos ? KFI_WEIGHT_REGULAR : name.mid(commaPos+2);
}

static bool      isSystem=false;
static Folder    theFolders[FontInst::FOLDER_COUNT];
static const int constSystemReconfigured=-1;
static const int constConnectionsTimeout = 30 * 1000;
static const int constFontListTimeout    = 10 * 1000;

typedef void (*SignalHandler)(int);

static void registerSignalHandler(SignalHandler handler)
{
    if (!handler)
        handler = SIG_DFL;

    sigset_t mask;
    sigemptyset(&mask);

#ifdef SIGSEGV
    signal(SIGSEGV, handler);
    sigaddset(&mask, SIGSEGV);
#endif
#ifdef SIGFPE
    signal(SIGFPE, handler);
    sigaddset(&mask, SIGFPE);
#endif
#ifdef SIGILL
    signal(SIGILL, handler);
    sigaddset(&mask, SIGILL);
#endif
#ifdef SIGABRT
    signal(SIGABRT, handler);
    sigaddset(&mask, SIGABRT);
#endif

    sigprocmask(SIG_UNBLOCK, &mask, nullptr);
}

void signalHander(int)
{
    static bool inHandler=false;
    
    if(!inHandler)
    {
        inHandler=true;
        theFolders[isSystem ? FontInst::FOLDER_SYS : FontInst::FOLDER_USER].saveDisabled();
        inHandler=false;
    }
}

FontInst::FontInst()
{
    isSystem=Misc::root();
    registerTypes();

    new FontinstAdaptor(this);
    QDBusConnection bus=QDBusConnection::sessionBus();

    KFI_DBUG << "Connecting to session bus";
    if(!bus.registerService(OrgKdeFontinstInterface::staticInterfaceName()))
    {
        KFI_DBUG << "Failed to register service!";
        ::exit(-1);
    }
    if(!bus.registerObject(FONTINST_PATH, this))
    {
        KFI_DBUG << "Failed to register object!";
        ::exit(-1);
    }

    registerSignalHandler(signalHander);
    itsConnectionsTimer=new QTimer(this);
    itsFontListTimer=new QTimer(this);
    connect(itsConnectionsTimer, &QTimer::timeout, this, &FontInst::connectionsTimeout);
    connect(itsFontListTimer, &QTimer::timeout, this, &FontInst::fontListTimeout);
    itsConnectionsTimer->start(constConnectionsTimeout);
    itsFontListTimer->start(constFontListTimeout);

    for(int i=0; i<(isSystem ? 1 : FOLDER_COUNT); ++i)
        theFolders[i].init(FOLDER_SYS==i, isSystem);

    updateFontList(false);
}

FontInst::~FontInst()
{
    for(int i=0; i<(isSystem ? 1 : FOLDER_COUNT); ++i)
        theFolders[i].saveDisabled();
}

void FontInst::list(int folders, int pid)
{
    KFI_DBUG << folders << pid;

    itsConnections.insert(pid);
    updateFontList(false);
    QList<KFI::Families> fonts;

    for(int i=0; i<(isSystem ? 1 : FOLDER_COUNT); ++i)
        if(0==folders || folders&(1<<i))
            fonts+=theFolders[i].list();

    itsConnectionsTimer->start(constConnectionsTimeout);
    itsFontListTimer->start(constFontListTimeout);
    emit fontList(pid, fonts);
}

void FontInst::statFont(const QString &name, int folders, int pid)
{
    KFI_DBUG << name << folders << pid;

    bool                      checkSystem=0==folders || folders&SYS_MASK || isSystem,
                              checkUser=0==folders || (folders&USR_MASK && !isSystem);
    FamilyCont::ConstIterator fam;
    StyleCont::ConstIterator  st;

    itsConnections.insert(pid);
    if( (checkSystem && findFont(name, FOLDER_SYS, fam, st)) ||
        (checkUser && findFont(name, FOLDER_USER, fam, st, !checkSystem)) )
    {
        Family rv((*fam).name());
        rv.add(*st);
        KFI_DBUG << "Found font, emit details...";
        emit fontStat(pid, rv);
    }
    else
    {
        KFI_DBUG << "Font not found, emit empty details...";
        emit fontStat(pid, Family(name));
    }
}

void FontInst::install(const QString &file, bool createAfm, bool toSystem, int pid, bool checkConfig)
{
    KFI_DBUG << file << createAfm << toSystem << pid << checkConfig;

    itsConnections.insert(pid);

    if(checkConfig)
        updateFontList();

    EFolder          folder=isSystem || toSystem ? FOLDER_SYS : FOLDER_USER;
    Family           font;
    Utils::EFileType type=Utils::check(file, font);

    int result=Utils::FILE_BITMAP==type && !FC::bitmapsEnabled()
                    ? STATUS_BITMAPS_DISABLED
                    : Utils::FILE_INVALID==type
                        ? STATUS_NOT_FONT_FILE
                        : STATUS_OK;

    if(STATUS_OK==result)
    {
        if(Utils::FILE_AFM!=type && Utils::FILE_PFM!=type)
            for(int i=0; i<(isSystem ? 1 : FOLDER_COUNT) && STATUS_OK==result; ++i)
                if(theFolders[i].contains(font.name(), (*font.styles().begin()).value()))
                    result=STATUS_ALREADY_INSTALLED;

        if(STATUS_OK==result)
        {
            QString name(Misc::modifyName(Misc::getFile(file))),
                    destFolder(Misc::getDestFolder(theFolders[folder].location(), name));

            result=Utils::FILE_AFM!=type && Utils::FILE_PFM!=type && Misc::fExists(destFolder+name) ? (int)KIO::ERR_FILE_ALREADY_EXIST : (int)STATUS_OK;
            if(STATUS_OK==result)
            {
                if(toSystem && !isSystem)
                {
                    KFI_DBUG << "Send request to system helper" << file << destFolder << name;
                    QVariantMap args;
                    args["method"] = "install";
                    args["file"] = file;
                    args["name"] = name;
                    args["destFolder"] = destFolder;
                    args["createAfm"] = createAfm;
                    args["type"] = (int)type;
                    result=performAction(args);
                }
                else
                {
                    if(!Misc::dExists(destFolder))
                        result=Misc::createDir(destFolder) ? (int)STATUS_OK : (int)KIO::ERR_WRITE_ACCESS_DENIED;

                    if(STATUS_OK==result)
                        result=QFile::copy(file, destFolder+name) ? (int)STATUS_OK : (int)KIO::ERR_WRITE_ACCESS_DENIED;

                    if(STATUS_OK==result)
                    {
                        Misc::setFilePerms(QFile::encodeName(destFolder+name));
                        if((Utils::FILE_SCALABLE==type || Utils::FILE_PFM==type) && createAfm)
                            Utils::createAfm(destFolder+name, type);
                    }
                }

                if(STATUS_OK==result && Utils::FILE_AFM!=type && Utils::FILE_PFM!=type)
                {
                    StyleCont::ConstIterator st(font.styles().begin());
                    FileCont::ConstIterator  f((*st).files().begin());
                    File                     df(destFolder+name, (*f).foundry(), (*f).index());

                    (*st).clearFiles();
                    (*st).add(df);
                    theFolders[folder].add(font);
                    theFolders[folder].addModifiedDir(destFolder);
                    emit fontsAdded(Families(font, FOLDER_SYS==folder));
                }
            }
        }
    }

    emit status(pid, result);
    itsConnectionsTimer->start(constConnectionsTimeout);
    itsFontListTimer->start(constFontListTimeout);
}

void FontInst::uninstall(const QString &family, quint32 style, bool fromSystem, int pid, bool checkConfig)
{
    KFI_DBUG << family << style << fromSystem << pid << checkConfig;

    itsConnections.insert(pid);

    if(checkConfig)
        updateFontList();

    EFolder                   folder=isSystem || fromSystem ? FOLDER_SYS : FOLDER_USER;
    FamilyCont::ConstIterator fam;
    StyleCont::ConstIterator  st;
    int                       result=findFont(family, style, folder, fam, st) ? (int)STATUS_OK : (int)KIO::ERR_DOES_NOT_EXIST;

    if(STATUS_OK==result)
    {
        Family                  del((*fam).name());
        Style                   s((*st).value(), (*st).scalable(), (*st).writingSystems());
        FileCont                files((*st).files());
        FileCont::ConstIterator it(files.begin()),
                                end(files.end());

        if(fromSystem && !isSystem)
        {
            QStringList fileList;
            bool        wasDisabled(false);

            for(; it!=end; ++it)
            {
                fileList.append((*it).path());
                theFolders[FOLDER_SYS].addModifiedDir(Misc::getDir((*it).path()));
                if(!wasDisabled && Misc::isHidden(Misc::getFile((*it).path())))
                    wasDisabled=true;
            }
            QVariantMap args;
            args["method"] = "uninstall";
            args["files"] = fileList;
            result=performAction(args);

            if(STATUS_OK==result)
            {
                FileCont empty;
                s.setFiles(files);
                (*st).setFiles(empty);
                if(wasDisabled)
                    theFolders[FOLDER_SYS].setDisabledDirty();
            }
        }
        else
        {
            for(; it!=end; ++it)
                if(!Misc::fExists((*it).path()) || QFile::remove((*it).path()))
                {
                    // Also remove any AFM or PFM files...
                    QStringList other;
                    Misc::getAssociatedFiles((*it).path(), other);
                    QStringList::ConstIterator oit(other.constBegin()),
                                                oend(other.constEnd());
                    for(; oit!=oend; ++oit)
                        QFile::remove(*oit);

                    theFolders[folder].addModifiedDir(Misc::getDir((*it).path()));
                    (*st).remove(*it);
                    s.add(*it);
                    if(!theFolders[folder].disabledDirty() && Misc::isHidden(Misc::getFile((*it).path())))
                        theFolders[folder].setDisabledDirty();
                }
        }

        if(STATUS_OK==result)
        {
            if((*st).files().isEmpty())
            {
                (*fam).remove(*st);
                if((*fam).styles().isEmpty())
                    theFolders[folder].removeFont(*fam);
            }
            else
                result=STATUS_PARTIAL_DELETE;
            del.add(s);
        }
        emit fontsRemoved(Families(del, FOLDER_SYS==folder));

    }
    KFI_DBUG << "status" << result;
    emit status(pid, result);

    itsConnectionsTimer->start(constConnectionsTimeout);
    itsFontListTimer->start(constFontListTimeout);
}

void FontInst::uninstall(const QString &name, bool fromSystem, int pid, bool checkConfig)
{
    KFI_DBUG << name << fromSystem  << pid << checkConfig;

    FamilyCont::ConstIterator fam;
    StyleCont::ConstIterator  st;
    if(findFont(name, fromSystem || isSystem ? FOLDER_SYS : FOLDER_USER, fam, st))
        uninstall((*fam).name(), (*st).value(), fromSystem, pid, checkConfig);
    else
        emit status(pid, KIO::ERR_DOES_NOT_EXIST);
}

void FontInst::move(const QString &family, quint32 style, bool toSystem, int pid, bool checkConfig)
{
    KFI_DBUG << family << style << toSystem << pid << checkConfig;

    itsConnections.insert(pid);
    if(checkConfig)
        updateFontList();

    if(isSystem)
        emit status(pid, KIO::ERR_UNSUPPORTED_ACTION);
    else
    {
        FamilyCont::ConstIterator fam;
        StyleCont::ConstIterator  st;
        EFolder                   from=toSystem ? FOLDER_USER : FOLDER_SYS,
                                  to=toSystem ? FOLDER_SYS : FOLDER_USER;

        if(findFont(family, style, from, fam, st))
        {
            FileCont::ConstIterator it((*st).files().begin()),
                                    end((*st).files().end());
            QStringList files;

            for(; it!=end; ++it)
            {
                files.append((*it).path());
                theFolders[from].addModifiedDir(Misc::getDir((*it).path()));
                // Actual 'to' folder does not really matter, as we only want to call fc-cache
                // ...actual folders only matter for xreating fonts.dir, etc, and we wont be doing this...
                theFolders[to].addModifiedDir(theFolders[to].location());
            }

            QVariantMap args;
            args["method"] = "move";
            args["files"] = files;
            args["toSystem"] = toSystem;
            args["dest"] = theFolders[to].location();
            args["uid"] = getuid();
            args["gid"] = getgid();
            int result=performAction(args);

            if(STATUS_OK==result)
                updateFontList();
            emit status(pid, result);
        }
        else
        {
            KFI_DBUG << "does not exist";
            emit status(pid, KIO::ERR_DOES_NOT_EXIST);
        }
    }

    itsConnectionsTimer->start(constConnectionsTimeout);
    itsFontListTimer->start(constFontListTimeout);
}

static bool renameFontFile(const QString &from, const QString &to, int uid=-1, int gid=-1)
{
    if (!QFile::rename(from, to))
        return false;

    QByteArray dest(QFile::encodeName(to));
    Misc::setFilePerms(dest);
    if(-1!=uid && -1!=gid)
        ::chown(dest.data(), uid, gid);
    return true;
}

void FontInst::enable(const QString &family, quint32 style, bool inSystem, int pid, bool checkConfig)
{
    KFI_DBUG << family << style << inSystem << pid << checkConfig;
    toggle(true, family, style, inSystem, pid, checkConfig);
}

void FontInst::disable(const QString &family, quint32 style, bool inSystem, int pid, bool checkConfig)
{
    KFI_DBUG << family << style << inSystem << pid << checkConfig;
    toggle(false, family, style, inSystem, pid, checkConfig);
}

void FontInst::removeFile(const QString &family, quint32 style, const QString &file, bool fromSystem, int pid,
                          bool checkConfig)
{
    KFI_DBUG << family << style << file << fromSystem << pid << checkConfig;

    itsConnections.insert(pid);

    if(checkConfig)
        updateFontList();

    // First find the family/style
    EFolder                   folder=isSystem || fromSystem ? FOLDER_SYS : FOLDER_USER;
    FamilyCont::ConstIterator fam;
    StyleCont::ConstIterator  st;
    int                       result=findFont(family, style, folder, fam, st) ? (int)STATUS_OK : (int)KIO::ERR_DOES_NOT_EXIST;

    if(STATUS_OK==result)
    {
        // Family/style found - now check that the requested file is *within* the same folder as one
        // of the files linked to this font...
        FileCont                files((*st).files());
        FileCont::ConstIterator it(files.begin()),
                                end(files.end());
        QString                 dir(Misc::getDir(file));

        result=KIO::ERR_DOES_NOT_EXIST;
        for(; it!=end && STATUS_OK!=result; ++it)
            if(Misc::getDir((*it).path())==dir)
                result=STATUS_OK;

        if(STATUS_OK==result)
        {
            // OK, found folder - so can now proceed to delete the file...
            if(fromSystem && !isSystem)
            {
                QVariantMap args;
                args["method"] = "removeFile";
                args["file"] = file;
                result=performAction(args);
            }
            else
            {
                result=Misc::fExists(file)
                        ? QFile::remove(file)
                            ? (int)STATUS_OK
                            : (int)KIO::ERR_WRITE_ACCESS_DENIED
                        : (int)KIO::ERR_DOES_NOT_EXIST;
            }

            if(STATUS_OK==result)
                theFolders[folder].addModifiedDir(dir);
        }
    }

    emit status(pid, result);
}

void FontInst::reconfigure(int pid, bool force)
{
    KFI_DBUG << pid << force;
    bool sysModified(theFolders[FOLDER_SYS].isModified());

    saveDisabled();

    KFI_DBUG << theFolders[FOLDER_USER].isModified() << sysModified;
    if(!isSystem && (force || theFolders[FOLDER_USER].isModified()))
        theFolders[FOLDER_USER].configure(force);

    if(sysModified)
    {
        if(isSystem)
        {
            theFolders[FOLDER_SYS].configure();
        }
        else
        {
            QVariantMap args;
            args["method"] = "reconfigure";
            performAction(args);
            theFolders[FOLDER_SYS].clearModified();
        }
    }

    itsConnectionsTimer->start(constConnectionsTimeout);
    itsFontListTimer->start(constFontListTimeout);

    updateFontList();
    emit status(pid, isSystem ? constSystemReconfigured : STATUS_OK);
}

QString FontInst::folderName(bool sys)
{
    return theFolders[sys || isSystem ? FOLDER_SYS : FOLDER_USER].location();
}

void FontInst::saveDisabled()
{
    if(isSystem)
        theFolders[FOLDER_SYS].saveDisabled();
    else
        for(int i=0; i<(isSystem ? 1 : FOLDER_COUNT); ++i)
            if(FOLDER_SYS==i && !isSystem)
            {
                if(theFolders[i].disabledDirty())
                {
                    QVariantMap args;
                    args["method"] = "saveDisabled";
                    performAction(args);
                    theFolders[i].saveDisabled();
                }
            }
            else
                theFolders[i].saveDisabled();
}

void FontInst::connectionsTimeout()
{
    bool canExit(true);

    KFI_DBUG << "exiting";
    checkConnections();

    for(int i=0; i<(isSystem ? 1 : FOLDER_COUNT); ++i)
    {
        if(theFolders[i].disabledDirty())
            canExit=false;
        theFolders[i].saveDisabled();
    }

    if(0==itsConnections.count())
    {
        if(canExit)
            qApp->exit(0);
        else // Try again later...
            itsConnectionsTimer->start(constConnectionsTimeout);
    }
}

void FontInst::fontListTimeout()
{
    updateFontList(true);
    itsFontListTimer->start(constFontListTimeout);
}

void FontInst::updateFontList(bool emitChanges)
{
    // For some reason just the "!FcConfigUptoDate(0)" check does not always work :-(
    FcBool fcModified=!FcConfigUptoDate(nullptr);

    if(fcModified ||
       theFolders[FOLDER_SYS].fonts().isEmpty() ||
       (!isSystem && theFolders[FOLDER_USER].fonts().isEmpty()) ||
       theFolders[FOLDER_SYS].disabledDirty() ||
       (!isSystem && theFolders[FOLDER_USER].disabledDirty()))
    {
        KFI_DBUG << "Need to refresh font lists";
        if(fcModified)
        {
            KFI_DBUG << "Re-init FC";
            if(!FcInitReinitialize())
                KFI_DBUG << "Re-init failed????";
        }

        Folder::Flat old[FOLDER_COUNT];

        if(emitChanges)
        {
            KFI_DBUG << "Flatten existing font lists";
            for(int i=0; i<(isSystem ? 1 : FOLDER_COUNT); ++i)
                old[i]=theFolders[i].flatten();
        }

        saveDisabled();

        for(int i=0; i<(isSystem ? 1 : FOLDER_COUNT); ++i)
            theFolders[i].clearFonts();

        KFI_DBUG << "update list of fonts";

        FcPattern   *pat = FcPatternCreate();
        FcObjectSet *os  = FcObjectSetBuild(FC_FILE, FC_FAMILY, FC_FAMILYLANG,
                                            FC_WEIGHT, FC_LANG, FC_CHARSET, FC_SCALABLE,
#ifndef KFI_FC_NO_WIDTHS
                                            FC_WIDTH,
#endif
                                            FC_SLANT, FC_INDEX, FC_FOUNDRY, (void*)nullptr);

        FcFontSet   *list=FcFontList(nullptr, pat, os);

        FcPatternDestroy(pat);
        FcObjectSetDestroy(os);

        theFolders[FOLDER_SYS].loadDisabled();
        if(!isSystem)
            theFolders[FOLDER_USER].loadDisabled();

        if(list)
        {
            QString home(Misc::dirSyntax(QDir::homePath()));

            for (int i = 0; i < list->nfont; i++)
            {
                QString fileName(Misc::fileSyntax(FC::getFcString(list->fonts[i], FC_FILE)));

                if(!fileName.isEmpty() && Misc::fExists(fileName)) // && 0!=fileName.indexOf(constDefomaLocation))
                {
                    QString    family,
                               foundry;
                    quint32    styleVal;
                    int        index;
                    qulonglong writingSystems(WritingSystems::instance()->get(list->fonts[i]));
                    FcBool     scalable=FcFalse;

                    if(FcResultMatch!=FcPatternGetBool(list->fonts[i], FC_SCALABLE, 0, &scalable))
                        scalable=FcFalse;

                    FC::getDetails(list->fonts[i], family, styleVal, index, foundry);
                    FamilyCont::ConstIterator fam=theFolders[isSystem || 0!=fileName.indexOf(home)
                                                                ? FOLDER_SYS : FOLDER_USER].addFont(Family(family));
                    StyleCont::ConstIterator  style=(*fam).add(Style(styleVal));
                    FileCont::ConstIterator   file=(*style).add(File(fileName, foundry, index));
                    Q_UNUSED(file);

                    (*style).setWritingSystems((*style).writingSystems()|writingSystems);
                    if(scalable)
                        (*style).setScalable();
                }
            }

            FcFontSetDestroy(list);
        }

        if(emitChanges)
        {
            KFI_DBUG << "Look for differences";
            for(int i=0; i<(isSystem ? 1 : FOLDER_COUNT); ++i)
            {
                KFI_DBUG << "Flatten, and take copies...";
                Folder::Flat newList=theFolders[i].flatten(),
                             onlyNew=newList;

                KFI_DBUG << "Determine differences...";
                onlyNew.subtract(old[i]);
                old[i].subtract(newList);

                KFI_DBUG << "Emit changes...";
                Families families=onlyNew.build(isSystem || i==FOLDER_SYS);

                if(!families.items.isEmpty())
                    emit fontsAdded(families);

                families=old[i].build(isSystem || i==FOLDER_SYS);
                if(!families.items.isEmpty())
                    emit fontsRemoved(families);
            }
        }
        KFI_DBUG << "updated list of fonts";
    }
}

void FontInst::toggle(bool enable, const QString &family, quint32 style, bool inSystem, int pid, bool checkConfig)
{
    KFI_DBUG;
    itsConnections.insert(pid);

    if(checkConfig)
        updateFontList();

    EFolder                   folder=isSystem || inSystem ? FOLDER_SYS : FOLDER_USER;
    FamilyCont::ConstIterator fam;
    StyleCont::ConstIterator  st;
    int                       result=findFont(family, style, folder, fam, st) ? (int)STATUS_OK : (int)KIO::ERR_DOES_NOT_EXIST;

    if(STATUS_OK==result)
    {
        FileCont                  files((*st).files()),
                                  toggledFiles;
        FileCont::ConstIterator   it(files.begin()),
                                  end(files.end());
        QHash<File, QString>      movedFonts;
        QHash<QString, QString>   movedAssoc;
        QSet<QString>             modifiedDirs;

        for(; it!=end && STATUS_OK==result; ++it)
        {
            QString to=Misc::getDir((*it).path())+
                            QString(enable ? Misc::unhide(Misc::getFile((*it).path()))
                                           : Misc::hide(Misc::getFile((*it).path())));
            if(to!=(*it).path())
            {
                KFI_DBUG << "MOVE:" << (*it).path() << " to " << to;
                // If the font is a system font, and we're not root, then just go through the actions here - so
                // that we can build the list of changes that would happen...
                if((inSystem && !isSystem) || renameFontFile((*it).path(), to))
                {
                    modifiedDirs.insert(Misc::getDir(enable ? to : (*it).path()));
                    toggledFiles.insert(File(to, (*it).foundry(), (*it).index()));
                    // Now try to move an associated AFM or PFM files...
                    QStringList assoc;

                    movedFonts[*it]=to;
                    Misc::getAssociatedFiles((*it).path(), assoc);

                    QStringList::ConstIterator ait(assoc.constBegin()),
                                               aend(assoc.constEnd());

                    for(; ait!=aend && STATUS_OK==result; ++ait)
                    {
                        to=Misc::getDir(*ait)+
                                QString(enable ? Misc::unhide(Misc::getFile(*ait))
                                               : Misc::hide(Misc::getFile(*ait)));

                        if(to!=*ait)
                        {
                            if((inSystem && !isSystem) || renameFontFile(*ait, to))
                            {
                                movedAssoc[*ait]=to;
                            }
                            else
                            {
                                result=KIO::ERR_WRITE_ACCESS_DENIED;
                            }
                        }
                    }
                }
                else
                {
                    result=KIO::ERR_WRITE_ACCESS_DENIED;
                }
            }
        }

        if(inSystem && !isSystem)
        {
            Family toggleFam((*fam).name());

            toggleFam.add(*st);
            QVariantMap args;
            args["method"] = "toggle";
            QString     xml;
            QTextStream str(&xml);
            toggleFam.toXml(false, str);
            args["xml"] = xml;
            args["enable"] = enable;
            result=performAction(args);
        }

        if(STATUS_OK==result)
        {
            Family addFam((*fam).name()),
                delFam((*fam).name());
            Style  addStyle((*st).value(), (*st).scalable(), (*st).writingSystems()),
                delStyle((*st).value(), (*st).scalable(), (*st).writingSystems());

            addStyle.setFiles(toggledFiles);
            addFam.add(addStyle);
            delStyle.setFiles(files);
            delFam.add(delStyle);
            (*st).setFiles(toggledFiles);

            theFolders[folder].addModifiedDirs(modifiedDirs);
            emit fontsAdded(Families(addFam, FOLDER_SYS==folder));
            emit fontsRemoved(Families(delFam, FOLDER_SYS==folder));

            theFolders[folder].setDisabledDirty();
        }
        else // un-move fonts!
        {
            QHash<File, QString>::ConstIterator    fit(movedFonts.constBegin()),
                                                   fend(movedFonts.constEnd());
            QHash<QString, QString>::ConstIterator ait(movedAssoc.constBegin()),
                                                   aend(movedAssoc.constEnd());

            for(; fit!=fend; ++fit)
                renameFontFile(fit.value(), fit.key().path());
            for(; ait!=aend; ++ait)
                renameFontFile(ait.value(), ait.key());
        }
    }
    emit status(pid, result);

    itsConnectionsTimer->start(constConnectionsTimeout);
    itsFontListTimer->start(constFontListTimeout);
}

void FontInst::addModifedSysFolders(const Family &family)
{
    StyleCont::ConstIterator style(family.styles().begin()),
                             styleEnd(family.styles().end());

    for(; style!=styleEnd; ++style)
    {
        FileCont::ConstIterator file((*style).files().begin()),
                                fileEnd((*style).files().end());

        for(; file!=fileEnd; ++file)
            theFolders[FOLDER_SYS].addModifiedDir(Misc::getDir((*file).path()));
    }
}

void FontInst::checkConnections()
{
    KFI_DBUG;
    QSet<int>::ConstIterator it(itsConnections.begin()),
                             end(itsConnections.end());
    QSet<int>                remove;

    for(; it!=end; ++it)
        if(0!=kill(*it, 0))
            remove.insert(*it);
    itsConnections.subtract(remove);
}

bool FontInst::findFontReal(const QString &family, const QString &style, EFolder folder,
                            FamilyCont::ConstIterator &fam, StyleCont::ConstIterator &st)
{
    KFI_DBUG;
    Family f(family);
    fam=theFolders[folder].fonts().find(f);
    if(theFolders[folder].fonts().end()==fam)
        return false;

    StyleCont::ConstIterator end((*fam).styles().end());
    for(st=(*fam).styles().begin(); st!=end; ++st)
        if(FC::createStyleName((*st).value())==style)
            return true;

    return false;
}

bool FontInst::findFont(const QString &font, EFolder folder,
                        FamilyCont::ConstIterator &fam, StyleCont::ConstIterator &st,
                        bool updateList)
{
    KFI_DBUG;
    QString family,
            style;

    decompose(font, family, style);

    if(!findFontReal(family, style, folder, fam, st))
    {
        if(updateList)
        {
            // Not found, so refresh font list and try again...
            updateFontList();
            return findFontReal(family, style, folder, fam, st);
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool FontInst::findFontReal(const QString &family, quint32 style, EFolder folder,
                            FamilyCont::ConstIterator &fam, StyleCont::ConstIterator &st)
{
    KFI_DBUG;
    fam=theFolders[folder].fonts().find(Family(family));

    if(theFolders[folder].fonts().end()==fam)
        return false;
    else
    {
        st=(*fam).styles().find(style);

        return (*fam).styles().end()!=st;
    }
}

bool FontInst::findFont(const QString &family, quint32 style, EFolder folder,
                        FamilyCont::ConstIterator &fam, StyleCont::ConstIterator &st,
                        bool updateList)
{
    KFI_DBUG;
    if(!findFontReal(family, style, folder, fam, st))
    {
        if(updateList)
        {
            // Not found, so refresh font list and try again...
            updateFontList();
            return findFontReal(family, style, folder, fam, st);
        }
        else
        {
            return false;
        }
    }
    return true;
}

int FontInst::performAction(const QVariantMap &args)
{
    KAuth::Action action("org.kde.fontinst.manage");

    action.setHelperId("org.kde.fontinst");
    action.setArguments(args);
    KFI_DBUG << "Call " << args["method"].toString() << " on helper";
    itsFontListTimer->stop();
    itsConnectionsTimer->stop();

    KAuth::ExecuteJob* j = action.execute();
    j->exec();
    if (j->error()) {
        qWarning() << "kauth action failed" <<  j->errorString() << j->errorText();
        //error is a KAuth::ActionReply::Error rest of this code expects KIO error codes which are extended by EStatus
        switch (j->error()) {
            case KAuth::ActionReply::Error::UserCancelledError:
                return KIO::ERR_USER_CANCELED;
            case KAuth::ActionReply::Error::AuthorizationDeniedError:
                /*fall through*/
            case KAuth::ActionReply::Error::NoSuchActionError:
                return KIO::ERR_CANNOT_AUTHENTICATE;
            default:
                return KIO::ERR_INTERNAL;
        }
        return KIO::ERR_INTERNAL;
    }
    KFI_DBUG << "Success!";
    return STATUS_OK;
}

}
