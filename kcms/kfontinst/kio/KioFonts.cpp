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

#include <QFile>
#include <QCoreApplication>
#include <QMimeDatabase>
#include <QDebug>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <KZip>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "KioFonts.h"
#include "KfiConstants.h"
#include "FontInstInterface.h"
#include "FontInst.h"
#include "Fc.h"
#include "Misc.h"
#include "XmlStrings.h"
#include "Family.h"
#include "Style.h"
#include "File.h"
#include "debug.h"

#define MAX_IPC_SIZE     (1024*32)
#define KFI_DBUG         qCDebug(KCM_KFONTINST_KIO) << '(' << time(NULL) << ')'

static const int constReconfigTimeout = 10;

extern "C" {

Q_DECL_EXPORT int kdemain(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: kio_" KFI_KIO_FONTS_PROTOCOL
                        " protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("kio_" KFI_KIO_FONTS_PROTOCOL);
    KFI::CKioFonts   slave(argv[2], argv[3]);

    slave.dispatchLoop();

    return 0;
}

}

namespace KFI
{

inline bool isSysFolder(const QString &folder)
{
    return i18n(KFI_KIO_FONTS_SYS)==folder || KFI_KIO_FONTS_SYS==folder;
}

inline bool isUserFolder(const QString &folder)
{
    return i18n(KFI_KIO_FONTS_USER)==folder || KFI_KIO_FONTS_USER==folder;
}

static CKioFonts::EFolder getFolder(const QStringList &list)
{
    if(list.size()>0)
    {
        QString folder=list[0];

        if(isSysFolder(folder))
            return CKioFonts::FOLDER_SYS;
        else if(isUserFolder(folder))
            return CKioFonts::FOLDER_USER;
        return CKioFonts::FOLDER_UNKNOWN;
    }

    return CKioFonts::FOLDER_ROOT;
}

static int getSize(const QString &file)
{
    QT_STATBUF buff;
    QByteArray      f(QFile::encodeName(file));

    if(-1!=QT_LSTAT(f.constData(), &buff))
    {
        if (S_ISLNK(buff.st_mode))
        {
            char buffer2[1000];
            int n=readlink(f.constData(), buffer2, 999);
            if(n!= -1)
                buffer2[n]='\0';

            if(-1==QT_STAT(f.constData(), &buff))
                return -1;
        }
        return buff.st_size;
    }

    return -1;
}

static bool writeAll(int fd, const char *buf, size_t len)
{
   while(len>0)
   {
      ssize_t written=write(fd, buf, len);
      if (written<0 && EINTR!=errno)
          return false;
      buf+=written;
      len-=written;
   }
   return true;
}

static bool isScalable(const QString &str)
{
    return Misc::checkExt(str, "ttf") || Misc::checkExt(str, "otf") || Misc::checkExt(str, "ttc") ||
           Misc::checkExt(str, "pfa") || Misc::checkExt(str, "pfb");
}

static const char * const constExtensions[]=
            {".ttf", KFI_FONTS_PACKAGE, ".otf", ".pfa", ".pfb", ".ttc",
             ".pcf", ".pcf.gz", ".bdf", ".bdf.gz", nullptr };

static QString removeKnownExtension(const QUrl &url)
{
    QString fname(url.fileName());
    int     pos;

    for(int i=0; constExtensions[i]; ++i)
        if(-1!=(pos=fname.lastIndexOf(QString::fromLatin1(constExtensions[i]), -1, Qt::CaseInsensitive)))
            return fname.left(pos);
    return fname;
}


CKioFonts::CKioFonts(const QByteArray &pool, const QByteArray &app)
         : KIO::SlaveBase(KFI_KIO_FONTS_PROTOCOL, pool, app)
         , itsInterface(new FontInstInterface())
         , itsTempDir(nullptr)
{
    KFI_DBUG;
}

CKioFonts::~CKioFonts()
{
    KFI_DBUG;
    delete itsInterface;
    delete itsTempDir;
}

void CKioFonts::listDir(const QUrl &url)
{
    KFI_DBUG << url;

    QStringList pathList(url.adjusted(QUrl::StripTrailingSlash).path().split(QLatin1Char('/'), Qt::SkipEmptyParts));
    EFolder       folder=Misc::root() ? FOLDER_SYS : getFolder(pathList);
    KIO::UDSEntry entry;
    int           size=0;

    switch(folder)
    {
        case FOLDER_ROOT:
            KFI_DBUG << "List root folder";
            size=2;
            totalSize(2);
            createUDSEntry(entry, FOLDER_SYS);
            listEntry(entry);
            createUDSEntry(entry, FOLDER_USER);
            listEntry(entry);
            break;
        case FOLDER_SYS:
        case FOLDER_USER:
            size=listFolder(entry, folder);
            break;
        default:
            break;
    }

    if(FOLDER_UNKNOWN!=folder)
    {
        finished();
    }
    else
        error(KIO::ERR_DOES_NOT_EXIST, url.toDisplayString());
}

void CKioFonts::put(const QUrl &url, int /*permissions*/, KIO::JobFlags /*flags*/)
{
    KFI_DBUG << url;
    QStringList pathList(url.adjusted(QUrl::StripTrailingSlash).path().split(QLatin1Char('/'), Qt::SkipEmptyParts));
    EFolder     folder(getFolder(pathList));

    if(!Misc::root() && FOLDER_ROOT==folder)
        error(KIO::ERR_SLAVE_DEFINED,
                i18n("Can only install fonts to either \"%1\" or \"%2\".",
                i18n(KFI_KIO_FONTS_USER), i18n(KFI_KIO_FONTS_SYS)));
    else if(Misc::isPackage(url.fileName()))
        error(KIO::ERR_SLAVE_DEFINED, i18n("You cannot install a fonts package directly.\n"
                                           "Please extract %1, and install the components individually.",
                                           url.toDisplayString()));
    else
    {
        if(!itsTempDir)
        {
            itsTempDir=new QTemporaryDir(QDir::tempPath() +
                                         QString::fromLatin1("/kio_fonts_")+QString::number(getpid()));
            itsTempDir->setAutoRemove(true);
        }

        QString tempFile(itsTempDir->filePath(url.fileName()));
        QFile   dest(tempFile);

        if (dest.open(QIODevice::WriteOnly))
        {
            int result;
            // Loop until we got 0 (end of data)
            do
            {
                QByteArray buffer;

                dataReq(); // Request for data
                result = readData(buffer);
                if(result > 0 && !writeAll(dest.handle(), buffer.constData(), buffer.size()))
                {
                    if(ENOSPC==errno) // disk full
                    {
                        error(KIO::ERR_DISK_FULL, dest.fileName());
                        result = -2; // means: remove dest file
                    }
                    else
                    {
                        error(KIO::ERR_CANNOT_WRITE, dest.fileName());
                        result = -1;
                    }
                }
            }
            while(result>0);

            if (result<0)
            {
                dest.close();
                ::exit(255);
            }

            handleResp(itsInterface->install(tempFile, Misc::root() || FOLDER_SYS==folder),
                       url.fileName(), tempFile, FOLDER_SYS==folder);
            QFile::remove(tempFile);
        }
        else
            error(EACCES==errno ? KIO::ERR_WRITE_ACCESS_DENIED : KIO::ERR_CANNOT_OPEN_FOR_WRITING, dest.fileName());
    }
}

void CKioFonts::get(const QUrl &url)
{
    KFI_DBUG << url;
    QStringList pathList(url.adjusted(QUrl::StripTrailingSlash).path().split(QLatin1Char('/'), QString::SkipEmptyParts));
    EFolder     folder(getFolder(pathList));
    Family      family(getFont(url, folder));

    if(!family.name().isEmpty() && 1==family.styles().count())
    {
        StyleCont::ConstIterator style(family.styles().begin());
        FileCont::ConstIterator  it((*style).files().begin()),
                                 end((*style).files().end());

        //
        // The thumbnail job always downloads non-local files to /tmp/... and passes this file name to
        // the thumbnail creator. However, in the case of fonts which are split among many files, this
        // wont work. Therefore, when the thumbnail code asks for the font to download, just return
        // the family and style info for enabled fonts, and the filename for disabled fonts. This way
        // the font-thumbnail creator can read this and just ask Xft/fontconfig for the font data.
        if("1"==metaData("thumbnail"))
        {
            QByteArray  array;
            QTextStream stream(&array, QIODevice::WriteOnly);

            emit mimeType("text/plain");

            bool hidden(true);

            for(; it!=end && hidden; ++it)
                if(!Misc::isHidden(Misc::getFile((*it).path())))
                    hidden=false;

            if(hidden)
            {
                //
                // OK, its a disabled font - if possible try to return the location of the font file
                // itself.
                bool found=false;
                it=(*style).files().begin();
                end=(*style).files().end();
                for(; it!=end && hidden; ++it)
                    if(isScalable((*it).path()))
                    {
                        KFI_DBUG << "hasMetaData(\"thumbnail\"), so return FILE: "
                                    << (*it).path() << " / " << (*it).index();
                        stream << KFI_PATH_KEY << (*it).path() << Qt::endl
                               << KFI_FACE_KEY << (*it).index() << Qt::endl;
                        found=true;
                        break;
                    }

                if(!found)
                {
                    KFI_DBUG << "hasMetaData(\"thumbnail\"), so return Url: " << url;
                    stream << url.toDisplayString();
                }
            }
            else
            {
                KFI_DBUG << "hasMetaData(\"thumbnail\"), so return DETAILS: " << family.name() << " / "
                         << (*style).value();

                stream << KFI_NAME_KEY << family.name() << Qt::endl
                       << KFI_STYLE_KEY << (*style).value() << Qt::endl;
            }

            totalSize(array.size());
            data(array);
            processedSize(array.size());
            data(QByteArray());
            processedSize(array.size());
            finished();
            KFI_DBUG << "Finished thumbnail...";
            return;
        }

        QSet<QString>   files;
        QString         realPath;
        QT_STATBUF buff;
        bool            multiple=false;

        for(; it!=end; ++it)
        {
            QStringList assoc;

            files.insert((*it).path());

            Misc::getAssociatedFiles((*it).path(), assoc);

            QStringList::ConstIterator ait(assoc.constBegin()),
                                       aend(assoc.constEnd());

            for(; ait!=aend; ++ait)
                files.insert(*ait);
        }

        if(1==files.count())
            realPath=(*files.begin());
        else   // Font is made up of multiple files - so create .zip of them all!
        {
            QTemporaryFile tmpFile;

            if(tmpFile.open())
            {
                KZip zip(tmpFile.fileName());

                tmpFile.setAutoRemove(false);
                realPath=tmpFile.fileName();

                if(zip.open(QIODevice::WriteOnly))
                {
                    QMap<QString, QString>                map=Misc::getFontFileMap(files);
                    QMap<QString, QString>::ConstIterator it(map.constBegin()),
                                                          end(map.constEnd());

                    for(; it!=end; ++it)
                        zip.addLocalFile(it.value(), it.key());

                    multiple=true;
                    zip.close();
                }
            }
        }

        QByteArray realPathC(QFile::encodeName(realPath));
        KFI_DBUG << "real: " << realPathC;

        if (-2==QT_STAT(realPathC.constData(), &buff))
            error(EACCES==errno ? KIO::ERR_ACCESS_DENIED : KIO::ERR_DOES_NOT_EXIST, url.toDisplayString());
        else if (S_ISDIR(buff.st_mode))
            error(KIO::ERR_IS_DIRECTORY, url.toDisplayString());
        else if (!S_ISREG(buff.st_mode))
            error(KIO::ERR_CANNOT_OPEN_FOR_READING, url.toDisplayString());
        else
        {
            int fd = QT_OPEN(realPathC.constData(), O_RDONLY);

            if (fd < 0)
                error(KIO::ERR_CANNOT_OPEN_FOR_READING, url.toDisplayString());
            else
            {
                // Determine the mimetype of the file to be retrieved, and emit it.
                // This is mandatory in all slaves (for KRun/BrowserRun to work).
                // This code can be optimized by using QFileInfo instead of buff above
                // and passing it to mimeTypeForFile() instead of realPath.
                QMimeDatabase db;
                emit mimeType(db.mimeTypeForFile(realPath).name());

                totalSize(buff.st_size);

                KIO::filesize_t processed=0;
                char            buffer[MAX_IPC_SIZE];
                QByteArray      array;

                while(1)
                {
                    int n=::read(fd, buffer, MAX_IPC_SIZE);

                    if (-1==n)
                    {
                        if (EINTR==errno)
                            continue;

                        error(KIO::ERR_CANNOT_READ, url.toDisplayString());
                        ::close(fd);
                        if(multiple)
                            ::unlink(realPathC);
                        return;
                    }
                    if (0==n)
                        break; // Finished

                    array=array.fromRawData(buffer, n);
                    data(array);
                    array.clear();

                    processed+=n;
                    processedSize(processed);
                }

                data(QByteArray());
                ::close(fd);
                processedSize(buff.st_size);
                finished();
            }
        }
        if(multiple)
            ::unlink(realPathC);
    }
    else
        error(KIO::ERR_CANNOT_READ, url.toDisplayString());
}

void CKioFonts::copy(const QUrl &, const QUrl &, int, KIO::JobFlags)
{
    error(KIO::ERR_SLAVE_DEFINED, i18n("Cannot copy fonts"));
}

void CKioFonts::rename(const QUrl &, const QUrl &, KIO::JobFlags)
{
    error(KIO::ERR_SLAVE_DEFINED, i18n("Cannot move fonts"));
}

void CKioFonts::del(const QUrl &url, bool isFile)
{
    KFI_DBUG << url;
    QStringList pathList(url.adjusted(QUrl::StripTrailingSlash).path().split(QLatin1Char('/'), QString::SkipEmptyParts));
    EFolder     folder(getFolder(pathList));
    QString     name(removeKnownExtension(url));

    if(!isFile)
        error(KIO::ERR_SLAVE_DEFINED, i18n("Only fonts may be deleted."));
    else if(!Misc::root() && FOLDER_ROOT==folder)
        error(KIO::ERR_SLAVE_DEFINED,
                i18n("Can only remove fonts from either \"%1\" or \"%2\".",
                i18n(KFI_KIO_FONTS_USER), i18n(KFI_KIO_FONTS_SYS)));
    else if(!name.isEmpty())
        handleResp(itsInterface->uninstall(name, Misc::root() || FOLDER_SYS==folder), name);
    else
        error(KIO::ERR_DOES_NOT_EXIST, url.toDisplayString());
}

void CKioFonts::stat(const QUrl &url)
{
    KFI_DBUG << url;

    QStringList pathList(url.adjusted(QUrl::StripTrailingSlash).path().split(QLatin1Char('/'), Qt::SkipEmptyParts));
    EFolder       folder=getFolder(pathList);
    KIO::UDSEntry entry;
    bool          ok=true;

    switch(pathList.count())
    {
        case 0:
            createUDSEntry(entry, FOLDER_ROOT);
            break;
        case 1:
            if(Misc::root())
                ok=createStatEntry(entry, url, FOLDER_SYS);
            else if(FOLDER_SYS==folder || FOLDER_USER==folder)
                createUDSEntry(entry, folder);
            else
            {
                error(KIO::ERR_SLAVE_DEFINED,
                        i18n("Please specify \"%1\" or \"%2\".",
                        i18n(KFI_KIO_FONTS_USER), i18n(KFI_KIO_FONTS_SYS)));
                return;
            }
            break;
        default:
            ok=createStatEntry(entry, url, folder);
    }

    if(ok)
    {
        statEntry(entry);
        finished();
    }
    else
    {
        error(KIO::ERR_DOES_NOT_EXIST, url.toDisplayString());
        return;
    }
}

void CKioFonts::special(const QByteArray &a)
{
    if(!a.isEmpty())
        error(KIO::ERR_UNSUPPORTED_ACTION, i18n("No special methods supported."));
    else
    {
        setTimeoutSpecialCommand(-1);
        itsInterface->reconfigure();
    }
}

int CKioFonts::listFolder(KIO::UDSEntry &entry, EFolder folder)
{
    KFI_DBUG << folder;

    int                       styleCount(0);
    KFI::Families             families(itsInterface->list(FOLDER_SYS==folder));
    FamilyCont::ConstIterator family(families.items.begin()),
                              end(families.items.end());

    KFI_DBUG << "Num families:" << families.items.count();

    for(; family!=end; ++family)
    {
        StyleCont::ConstIterator styleIt((*family).styles().begin()),
                                 styleEnd((*family).styles().end());

        styleCount+=(*family).styles().count();
        for(; styleIt!=styleEnd; ++styleIt)
        {
            createUDSEntry(entry, folder, *family, *styleIt);
            listEntry(entry);
        }
    }

    totalSize(styleCount);
    return styleCount;
}

QString CKioFonts::getUserName(uid_t uid)
{
    if (!itsUserCache.contains(uid))
    {
        struct passwd *user = getpwuid(uid);
        if(user)
            itsUserCache.insert(uid, QString::fromLatin1(user->pw_name));
        else
            return QString::number(uid);
    }
    return itsUserCache[uid];
}

QString CKioFonts::getGroupName(gid_t gid)
{
    if (!itsGroupCache.contains(gid))
    {
        struct group *grp = getgrgid(gid);
        if(grp)
            itsGroupCache.insert(gid, QString::fromLatin1(grp->gr_name));
        else
            return QString::number(gid);
    }
    return itsGroupCache[gid];
}

bool CKioFonts::createStatEntry(KIO::UDSEntry &entry, const QUrl &url, EFolder folder)
{
    Family fam(getFont(url, folder));

    if(!fam.name().isEmpty() && 1==fam.styles().count())
    {
        createUDSEntry(entry, folder, fam, *fam.styles().begin());
        return true;
    }

    return false;
}

void CKioFonts::createUDSEntry(KIO::UDSEntry &entry, EFolder folder)
{
    KFI_DBUG << QString(FOLDER_SYS==folder ? i18n(KFI_KIO_FONTS_SYS) : i18n(KFI_KIO_FONTS_USER));
    entry.clear();
    entry.fastInsert(KIO::UDSEntry::UDS_NAME, FOLDER_ROOT==folder || Misc::root()
                                            ? i18n("Fonts")
                                            : FOLDER_SYS==folder
                                                ? i18n(KFI_KIO_FONTS_SYS)
                                                : i18n(KFI_KIO_FONTS_USER));
    entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, !Misc::root() && FOLDER_SYS==folder ? 0444 : 0744);
    entry.fastInsert(KIO::UDSEntry::UDS_USER, Misc::root() || FOLDER_SYS==folder
                                            ? QString::fromLatin1("root")
                                            : getUserName(getuid()));
    entry.fastInsert(KIO::UDSEntry::UDS_GROUP, Misc::root() || FOLDER_SYS==folder
                                            ? QString::fromLatin1("root")
                                            : getGroupName(getgid()));
    entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    entry.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("inode/directory"));
}

bool CKioFonts::createUDSEntry(KIO::UDSEntry &entry, EFolder folder, const Family &family, const Style &style)
{
    int                     size=0;
    QString                 name(FC::createName(family.name(), style.value()));
    FileCont::ConstIterator file(style.files().begin()),
                            fileEnd(style.files().end());
    QList<File>             files;
    bool                    hidden=true,
                            haveExtraFiles=false;

    KFI_DBUG << name;

    for(; file!=fileEnd; ++file)
    {
        size+=getSize((*file).path());
        // TODO: Make scalable a property of the file?
        // Then isScalable() is not needed!!!
        if(isScalable((*file).path()))
            files.prepend(*file);
        else
            files.append(*file);

        if(hidden && !Misc::isHidden(Misc::getFile((*file).path())))
            hidden=false;

        QStringList assoc;
        Misc::getAssociatedFiles((*file).path(), assoc);

        QStringList::ConstIterator oit(assoc.constBegin()),
                                   oend(assoc.constEnd());

        if(!haveExtraFiles && !assoc.isEmpty())
            haveExtraFiles=true;

        for(; oit!=oend; ++oit)
            size+=getSize(*oit);
    }

    entry.clear();

    entry.fastInsert(KIO::UDSEntry::UDS_NAME, name);
    entry.fastInsert(KIO::UDSEntry::UDS_SIZE, size);
    entry.fastInsert(UDS_EXTRA_FC_STYLE, style.value());

    QList<File>::ConstIterator it(files.constBegin()),
                               end(files.constEnd());

    for(; it!=end; ++it)
    {
        QByteArray      cPath(QFile::encodeName((*it).path()));
        QT_STATBUF buff;

        if(-1!=QT_LSTAT(cPath, &buff))
        {
            QString fileName(Misc::getFile((*it).path())),
                    mt;
            int     dotPos(fileName.lastIndexOf('.'));
            QString extension(-1==dotPos ? QString() : fileName.mid(dotPos));

            if(QString::fromLatin1(".gz")==extension)
            {
                dotPos=fileName.lastIndexOf('.', dotPos-1);
                extension=-1==dotPos ? QString() : fileName.mid(dotPos);
            }

            if(QString::fromLatin1(".ttf")==extension || QString::fromLatin1(".ttc")==extension)
                mt="application/x-font-ttf";
            else if(QString::fromLatin1(".otf")==extension)
                mt="application/x-font-otf";
            else if(QString::fromLatin1(".pfa")==extension || QString::fromLatin1(".pfb")==extension)
                mt="application/x-font-type1";
            else if(QString::fromLatin1(".pcf.gz")==extension || QString::fromLatin1(".pcf")==extension)
                mt="application/x-font-pcf";
            else if(QString::fromLatin1(".bdf.gz")==extension || QString::fromLatin1(".bdf")==extension)
                mt="application/x-font-bdf";
            else
            {
                // File extension check failed, use QMimeDatabase to read contents...
                QMimeDatabase db;
                QMimeType mime = db.mimeTypeForFile((*it).path());
                QStringList patterns = mime.globPatterns();
                mt = mime.name();
                if(patterns.size()>0)
                    extension=(*patterns.begin()).remove("*");
            }

            entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, buff.st_mode&S_IFMT);
            entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, buff.st_mode&07777);
            entry.fastInsert(KIO::UDSEntry::UDS_MODIFICATION_TIME, buff.st_mtime);
            entry.fastInsert(KIO::UDSEntry::UDS_ACCESS_TIME, buff.st_atime);
            entry.fastInsert(KIO::UDSEntry::UDS_USER, getUserName(buff.st_uid));
            entry.fastInsert(KIO::UDSEntry::UDS_GROUP, getGroupName(buff.st_gid));
            entry.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, mt);

            if(hidden)
            {
                entry.fastInsert(KIO::UDSEntry::UDS_HIDDEN, 1);
                entry.fastInsert(UDS_EXTRA_FILE_NAME, (*it).path());
                entry.fastInsert(UDS_EXTRA_FILE_FACE, (*it).path());
            }

            QString path(QString::fromLatin1("/"));

            if(!Misc::root())
            {
                path+=FOLDER_SYS==folder ? i18n(KFI_KIO_FONTS_SYS) : i18n(KFI_KIO_FONTS_USER);
                path+=QString::fromLatin1("/");
            }
            path+=name;

            if(files.count()>1 || haveExtraFiles)
                path+=QString::fromLatin1(KFI_FONTS_PACKAGE);
            else
                path+=extension;

            QUrl url(QUrl::fromLocalFile(path));
            url.setScheme(KFI_KIO_FONTS_PROTOCOL);
            entry.fastInsert(KIO::UDSEntry::UDS_URL, url.url());
            return true;
        }
    }

    return false;
}

Family CKioFonts::getFont(const QUrl &url, EFolder folder)
{
    QString name(removeKnownExtension(url));

    KFI_DBUG << url << name;

    return itsInterface->statFont(name, FOLDER_SYS==folder);
}

void CKioFonts::handleResp(int resp, const QString &file, const QString &tempFile, bool destIsSystem)
{
    switch(resp)
    {
        case FontInst::STATUS_NO_SYS_CONNECTION:
            error(KIO::ERR_SLAVE_DEFINED, i18n("Failed to start the system daemon"));
            break;
        case FontInst::STATUS_SERVICE_DIED:
            error(KIO::ERR_SLAVE_DEFINED, i18n("Backend died"));
            break;
        case FontInst::STATUS_BITMAPS_DISABLED:
            error(KIO::ERR_SLAVE_DEFINED,
                  i18n("%1 is a bitmap font, and these have been disabled on your system.", file));
            break;
        case FontInst::STATUS_ALREADY_INSTALLED:
            error(KIO::ERR_SLAVE_DEFINED,
                  i18n("%1 contains the font <b>%2</b>, which is already installed on your system.", file,
                  FC::getName(tempFile)));
            break;
        case FontInst::STATUS_NOT_FONT_FILE:
            error(KIO::ERR_SLAVE_DEFINED, i18n("%1 is not a font.", file));
            break;
        case FontInst::STATUS_PARTIAL_DELETE:
            error(KIO::ERR_SLAVE_DEFINED, i18n("Could not remove all files associated with %1", file));
            break;
        case KIO::ERR_FILE_ALREADY_EXIST:
        {
            QString name(Misc::modifyName(file)),
                    destFolder(Misc::getDestFolder(itsInterface->folderName(destIsSystem), name));
            error(KIO::ERR_SLAVE_DEFINED, i18n("<i>%1</i> already exists.", destFolder+name));
            break;
        }
        case FontInst::STATUS_OK:
            finished();
            break;
        default:
            error(resp, file);
    }

    if(FontInst::STATUS_OK==resp)
        setTimeoutSpecialCommand(constReconfigTimeout);
}

}
