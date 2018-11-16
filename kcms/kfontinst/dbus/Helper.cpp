/*
 * KHelper - KDE Font Installer
 *
 * Copyright 2003-2010 Craig Drummond <craig@kde.org>
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

#include "Helper.h"
#include "Folder.h"
#include "FontInst.h"
#include "Misc.h"
#include "Utils.h"
#include <QDebug>
#include <QCoreApplication>
#include <QDomDocument>
#include <QTextCodec>
#include <kio/global.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/errno.h>

#define KFI_DBUG qDebug() << time(nullptr)

KAUTH_HELPER_MAIN("org.kde.fontinst", KFI::Helper)

namespace KFI
{

static Folder theFontFolder;

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

static void signalHander(int)
{
    static bool inHandler=false;

    if(!inHandler)
    {
        inHandler=true;
        theFontFolder.saveDisabled();
        inHandler=false;
    }
}

static void cleanup()
{
    theFontFolder.saveDisabled();
}

Helper::Helper()
{
    KFI_DBUG;
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    registerSignalHandler(signalHander);
    qAddPostRoutine(cleanup);
    theFontFolder.init(true, true);
    theFontFolder.loadDisabled();
}

Helper::~Helper()
{
    theFontFolder.saveDisabled();
}

ActionReply Helper::manage(const QVariantMap &args)
{
    int result=KIO::ERR_UNSUPPORTED_ACTION;
    QString method=args["method"].toString();

    KFI_DBUG << method;

    if("install"==method)
        result=install(args);
    else if("uninstall"==method)
        result=uninstall(args);
    else if("move"==method)
        result=move(args);
    else if("toggle"==method)
        result=toggle(args);
    else if("removeFile"==method)
        result=removeFile(args);
    else if("reconfigure"==method)
        result=reconfigure();
    else if("saveDisabled"==method)
        result=saveDisabled();
    else
        KFI_DBUG << "Uknown action";

    if(FontInst::STATUS_OK==result)
        return ActionReply::SuccessReply();

    ActionReply reply(ActionReply::HelperErrorType);
    reply.setErrorCode(static_cast<KAuth::ActionReply::Error>(result));
    return reply;
}

int Helper::install(const QVariantMap &args)
{
    QString file(args["file"].toString()),
            name(args["name"].toString()),
            destFolder(args["destFolder"].toString());
    bool    createAfm(args["createAfm"].toBool());
    int     type(args["type"].toInt());

    KFI_DBUG << file << destFolder << name << createAfm;

    int result=FontInst::STATUS_OK;

    if(!Misc::dExists(destFolder))
        result=Misc::createDir(destFolder) ? (int)FontInst::STATUS_OK : (int)KIO::ERR_WRITE_ACCESS_DENIED;

    if(FontInst::STATUS_OK==result)
        result=QFile::copy(file, destFolder+name) ? (int)FontInst::STATUS_OK : (int)KIO::ERR_WRITE_ACCESS_DENIED;

    if(FontInst::STATUS_OK==result)
    {
        Misc::setFilePerms(QFile::encodeName(destFolder+name));
        if((Utils::FILE_SCALABLE==type || Utils::FILE_PFM==type) && createAfm)
            Utils::createAfm(destFolder+name, (KFI::Utils::EFileType)type);
        theFontFolder.addModifiedDir(destFolder);
    }

    return result;
}

int Helper::uninstall(const QVariantMap &args)
{
    QStringList files(args["files"].toStringList());
    int         result=checkWriteAction(files);

    if(FontInst::STATUS_OK==result)
    {
        QStringList::ConstIterator it(files.constBegin()),
                                   end(files.constEnd());

        for(; it!=end; ++it)
            if(!Misc::fExists(*it) || QFile::remove(*it))
            {
                // Also remove any AFM or PFM files...
                QStringList other;
                Misc::getAssociatedFiles(*it, other);
                QStringList::ConstIterator oit(other.constBegin()),
                                           oend(other.constEnd());
                for(; oit!=oend; ++oit)
                    QFile::remove(*oit);

                theFontFolder.addModifiedDir(Misc::getDir(*it));
            }
    }

    return result;
}

static bool renameFontFile(const QString &from, const QString &to, int uid=-1, int gid=-1)
{
    if(!QFile::rename(from, to))
        return false;

    QByteArray dest(QFile::encodeName(to));

    Misc::setFilePerms(dest);
    if(-1!=uid && -1!=gid)
        ::chown(dest.data(), uid, gid);
    return true;
}

int Helper::move(const QVariantMap &args)
{
    QStringList files(args["files"].toStringList());
    bool        toSystem(args["toSystem"].toBool());
    QString     dest(args["dest"].toString());
    int         uid(args["uid"].toInt()),
                gid(args["gid"].toInt());

    KFI_DBUG << files << dest << toSystem;

    int                        result=FontInst::STATUS_OK;
    QStringList::ConstIterator it(files.constBegin()),
                               end(files.constEnd());

    // Cant move hidden fonts - need to unhide first.
    for(; it!=end && FontInst::STATUS_OK==result; ++it)
        if(Misc::isHidden(Misc::getFile(*it)))
            result=KIO::ERR_UNSUPPORTED_ACTION;

    if(FontInst::STATUS_OK==result)
    {
        QHash<QString, QString> movedFiles;
        int                     toUid=toSystem ? getuid() : uid,
                                fromUid=toSystem ? uid : getuid(),
                                toGid=toSystem ? getgid() : gid,
                                fromGid=toSystem ? gid : getgid();

        // Move fonts!
        for(it=files.constBegin(); it!=end && FontInst::STATUS_OK==result; ++it)
        {
            QString name(Misc::modifyName(Misc::getFile(*it))),
                    destFolder(Misc::getDestFolder(dest, name));

            if(!Misc::dExists(destFolder))
            {
                result=Misc::createDir(destFolder) ? (int)FontInst::STATUS_OK : (int)KIO::ERR_WRITE_ACCESS_DENIED;
                if(FontInst::STATUS_OK==result)
                    ::chown(QFile::encodeName(destFolder).data(), toUid, toGid);
            }

            if(renameFontFile(*it, destFolder+name, toUid, toGid))
            {
                movedFiles[*it]=destFolder+name;
                // Now try to move an associated AFM or PFM files...
                QStringList assoc;

                Misc::getAssociatedFiles(*it, assoc);

                QStringList::ConstIterator ait(assoc.constBegin()),
                                           aend(assoc.constEnd());

                for(; ait!=aend && FontInst::STATUS_OK==result; ++ait)
                {
                    name=Misc::getFile(*ait);
                    if(renameFontFile(*ait, destFolder+name, toUid, toGid))
                        movedFiles[*ait]=destFolder+name;
                    else
                        result=KIO::ERR_WRITE_ACCESS_DENIED;
                }

                if(toSystem)
                    theFontFolder.addModifiedDir(theFontFolder.location());
            }
            else
                result=KIO::ERR_WRITE_ACCESS_DENIED;
        }

        if(FontInst::STATUS_OK!=result) // un-move fonts!
        {
            QHash<QString, QString>::ConstIterator it(movedFiles.constBegin()),
                                                   end(movedFiles.constEnd());

            for(; it!=end; ++it)
                renameFontFile(it.value(), it.key(), fromUid, fromGid);
        }
    }
    return result;
}

int Helper::toggle(const QVariantMap &args)
{
    QDomDocument    doc;
    doc.setContent(args["xml"].toString());
    Family          font(doc.documentElement(), true);
    bool            enable(args["enable"].toBool());

    KFI_DBUG << font.name() << enable;

    if(1!=font.styles().count())
        return KIO::ERR_WRITE_ACCESS_DENIED;

    int                     result=FontInst::STATUS_OK;
    FileCont                files((*font.styles().begin()).files()),
                            toggledFiles;
    FileCont::ConstIterator it(files.constBegin()),
                            end(files.constEnd());
    QHash<File, QString>    movedFonts;
    QHash<QString, QString> movedAssoc;
    QSet<QString>           modifiedDirs;
    // Move fonts!
    for(; it!=end && FontInst::STATUS_OK==result; ++it)
    {
        QString to=Misc::getDir((*it).path())+
                            QString(enable ? Misc::unhide(Misc::getFile((*it).path()))
                                           : Misc::hide(Misc::getFile((*it).path())));

        if(to!=(*it).path())
        {
            KFI_DBUG << "MOVE:" << (*it).path() << " to " << to;
            if(renameFontFile((*it).path(), to))
            {
                modifiedDirs.insert(Misc::getDir(enable ? to : (*it).path()));
                toggledFiles.insert(File(to, (*it).foundry(), (*it).index()));
                // Now try to move an associated AFM or PFM files...
                QStringList assoc;

                movedFonts[*it]=to;
                Misc::getAssociatedFiles((*it).path(), assoc);

                QStringList::ConstIterator ait(assoc.constBegin()),
                                           aend(assoc.constEnd());

                for(; ait!=aend && FontInst::STATUS_OK==result; ++ait)
                {
                    to=Misc::getDir(*ait)+
                            QString(enable ? Misc::unhide(Misc::getFile(*ait))
                                           : Misc::hide(Misc::getFile(*ait)));

                    if(to!=*ait) {
                        if(renameFontFile(*ait, to))
                            movedAssoc[*ait]=to;
                        else
                            result=KIO::ERR_WRITE_ACCESS_DENIED;
                    }
                }
            }
            else
                result=KIO::ERR_WRITE_ACCESS_DENIED;
        }
    }

    theFontFolder.addModifiedDirs(modifiedDirs);

    if(FontInst::STATUS_OK==result)
    {
        FamilyCont::ConstIterator f=theFontFolder.fonts().find(font);

        if(theFontFolder.fonts().end()==f)
            f=theFontFolder.addFont(font);

        StyleCont::ConstIterator st=(*f).styles().find(*font.styles().begin());

        if((*f).styles().end()==st)
            st=(*f).add(*font.styles().begin());

        // This helper only needs to store list of disabled fonts,
        // for writing back to disk - therefore no need to store
        // list of enabled font files.
        FileCont empty;
        (*st).setFiles(enable ? empty : toggledFiles);
        if((*st).files().isEmpty())
            (*f).remove(*st);
        if((*f).styles().isEmpty())
            theFontFolder.removeFont(*f);
        theFontFolder.setDisabledDirty();
    }
    else
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

    return result;
}

int Helper::removeFile(const QVariantMap &args)
{
    QString file(args["file"].toString());

    KFI_DBUG << file;

    QString dir(Misc::getDir(file));
    int     result=Misc::fExists(file)
                ? QFile::remove(file)
                    ? (int)FontInst::STATUS_OK
                    : (int)KIO::ERR_WRITE_ACCESS_DENIED
                : (int)KIO::ERR_DOES_NOT_EXIST;

    if(FontInst::STATUS_OK==result)
        theFontFolder.addModifiedDir(dir);

    return result;
}

int Helper::reconfigure()
{
    KFI_DBUG;

    saveDisabled();
    KFI_DBUG << theFontFolder.isModified();
    if(theFontFolder.isModified())
        theFontFolder.configure();
    return FontInst::STATUS_OK;
}

int Helper::saveDisabled()
{
    KFI_DBUG;
    // Load internally calls save!
    theFontFolder.loadDisabled();
    return FontInst::STATUS_OK;
}

int Helper::checkWriteAction(const QStringList &files)
{
    QStringList::ConstIterator it(files.constBegin()),
                               end(files.constEnd());

    for(; it!=end; ++it)
        if(!Misc::dWritable(Misc::getDir(*it)))
            return KIO::ERR_WRITE_ACCESS_DENIED;
    return FontInst::STATUS_OK;
}

}
