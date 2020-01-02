/**
 *  Copyright (c) Martin R. Jones 1996
 *  Copyright (c) Bernd Wuebben 1998
 *  Copyright (c) Christian Tibirna 1998
 *  Copyright 1998-2007 David Faure <faure@kde.org>
 *  Copyright (c) 2010 Matthias Fuchs <mat69@gmx.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

//
//
// "Desktop Options" Tab for KDesktop configuration
//
// (c) Martin R. Jones 1996
// (c) Bernd Wuebben 1998
//
// Layouts
// (c) Christian Tibirna 1998
// Port to KControl, split from Misc Tab, Port to KControl2
// (c) David Faure 1998
// Desktop menus, paths
// (c) David Faure 2000


// Own
#include "globalpaths.h"
#include "desktoppathssettings.h"

// Qt
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QFormLayout>
#include <QApplication>
#include <QLoggingCategory>
#include <QStandardPaths>

// KDE
#include <kconfiggroup.h>
#include <kfileitem.h>
#include <kio/copyjob.h>
#include <kio/deletejob.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kjobwidgets.h>
#include <KLocalizedString>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <KPluginFactory>
#include <KConfig>
#include <KSharedConfig>

Q_LOGGING_CATEGORY(KCM_DESKTOPPATH, "kcm_desktoppath")

K_PLUGIN_FACTORY(KcmDesktopPathsFactory, registerPlugin<DesktopPathConfig>();)

//-----------------------------------------------------------------------------

DesktopPathConfig::DesktopPathConfig(QWidget *parent, const QVariantList &)
    : KCModule( parent )
    , m_pathsSettings(new DesktopPathsSettings(this))
{
  QFormLayout *lay = new QFormLayout(this);
  lay->setVerticalSpacing(0);
  lay->setContentsMargins(0, 0, 0, 0);

  setQuickHelp( i18n("<h1>Paths</h1>\n"
    "This module allows you to choose where in the filesystem the "
    "files on your desktop should be stored.\n"
    "Use the \"Whats This?\" (Shift+F1) to get help on specific options."));

  urDesktop = addRow(lay, i18n("Desktop path:"),
                     i18n("This folder contains all the files"
                          " which you see on your desktop. You can change the location of this"
                          " folder if you want to, and the contents will move automatically"
                          " to the new location as well."));

  urAutostart = addRow(lay, i18n("Autostart path:"),
                       i18n("This folder contains applications or"
                            " links to applications (shortcuts) that you want to have started"
                            " automatically whenever the session starts. You can change the location of this"
                            " folder if you want to, and the contents will move automatically"
                            " to the new location as well."));

  urDocument = addRow(lay, i18n("Documents path:"),
                      i18n("This folder will be used by default to "
                           "load or save documents from or to."));

  urDownload = addRow(lay, i18n("Downloads path:"),
                      i18n("This folder will be used by default to "
                           "save your downloaded items."));

  urMovie = addRow(lay, i18n("Movies path:"),
                   i18n("This folder will be used by default to "
                        "load or save movies from or to."));

  urPicture = addRow(lay, i18n("Pictures path:"),
                     i18n("This folder will be used by default to "
                          "load or save pictures from or to."));

  urMusic = addRow(lay, i18n("Music path:"),
                   i18n("This folder will be used by default to "
                        "load or save music from or to."));
}

KUrlRequester* DesktopPathConfig::addRow(QFormLayout *lay, const QString& label, const QString& whatsThis)
{
    KUrlRequester* ur = new KUrlRequester(this);
    ur->setMode(KFile::Directory | KFile::LocalOnly);
    lay->addRow(label, ur);
    connect(ur, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    lay->labelForField(ur)->setWhatsThis(whatsThis);
    ur->setWhatsThis(whatsThis);
    return ur;
}

void DesktopPathConfig::load()
{
    m_pathsSettings->load();

    // Desktop Paths
    urAutostart->setUrl(m_pathsSettings->autostartLocation());

    urDesktop->setUrl(m_pathsSettings->desktopLocation());
    urDocument->setUrl(m_pathsSettings->documentsLocation());
    urDownload->setUrl(m_pathsSettings->downloadsLocation());
    urMovie->setUrl(m_pathsSettings->videosLocation());
    urPicture->setUrl(m_pathsSettings->picturesLocation());
    urMusic->setUrl(m_pathsSettings->musicLocation());

    emit changed(false);
}

void DesktopPathConfig::defaults()
{
    // Desktop Paths - keep defaults in sync with kglobalsettings.cpp
    m_pathsSettings->setDefaults();
    m_pathsSettings->setDefaults();

    urAutostart->setUrl(m_pathsSettings->autostartLocation());

    urDesktop->setUrl(m_pathsSettings->desktopLocation());
    urDocument->setUrl(m_pathsSettings->documentsLocation());
    urDownload->setUrl(m_pathsSettings->downloadsLocation());
    urMovie->setUrl(m_pathsSettings->videosLocation());
    urPicture->setUrl(m_pathsSettings->picturesLocation());
    urMusic->setUrl(m_pathsSettings->musicLocation());
}

void DesktopPathConfig::save()
{
    bool autostartMoved = false;

    QUrl desktopURL = m_pathsSettings->desktopLocation();

    QUrl autostartURL = m_pathsSettings->autostartLocation();
    QUrl newAutostartURL = urAutostart->url();

    if ( !urDesktop->url().matches( desktopURL, QUrl::StripTrailingSlash ) )
    {
        // Test which other paths were inside this one (as it is by default)
        // and for each, test where it should go.
        // * Inside destination -> let them be moved with the desktop (but adjust name if necessary)
        // * Not inside destination -> move first
        // !!!
        qCDebug(KCM_DESKTOPPATH) << "desktopURL=" << desktopURL;
        QString urlDesktop = urDesktop->url().toLocalFile();
        if ( !urlDesktop.endsWith(QLatin1Char('/')))
            urlDesktop+=QLatin1Char('/');

        if ( desktopURL.isParentOf( autostartURL ) )
        {
            qCDebug(KCM_DESKTOPPATH) << "Autostart is on the desktop";

            // Either the Autostart field wasn't changed (-> need to update it)
            if ( newAutostartURL.matches(autostartURL, QUrl::StripTrailingSlash) )
            {
                // Hack. It could be in a subdir inside desktop. Hmmm... Argl.
                urAutostart->setUrl(QUrl::fromLocalFile(urlDesktop + QStringLiteral("Autostart/")));
                qCDebug(KCM_DESKTOPPATH) << "Autostart is moved with the desktop";
                autostartMoved = true;
            }
            // or it has been changed (->need to move it from here)
            else
            {
                QUrl futureAutostartURL = QUrl::fromLocalFile(urlDesktop + QStringLiteral("Autostart/"));
                if ( newAutostartURL.matches( futureAutostartURL, QUrl::StripTrailingSlash ) )
                    autostartMoved = true;
                else
                    autostartMoved = moveDir( m_pathsSettings->autostartLocation(), urAutostart->url(), i18n("Autostart") );
            }
        }

        if ( moveDir( m_pathsSettings->desktopLocation(), QUrl::fromLocalFile( urlDesktop ), i18n("Desktop") ) )
        {
            //save in XDG path
            m_pathsSettings->setDesktopLocation(QUrl::fromLocalFile(urlDesktop));
        }
    }

    if ( !newAutostartURL.matches( autostartURL, QUrl::StripTrailingSlash ) )
    {
        if (!autostartMoved)
            autostartMoved = moveDir( m_pathsSettings->autostartLocation(), urAutostart->url(), i18n("Autostart") );
        if (autostartMoved)
        {
            m_pathsSettings->setAutostartLocation(urAutostart->url());
        }
    }

    xdgSavePath(urDocument, m_pathsSettings->documentsLocation(), "documentsLocation", i18n("Documents"));
    xdgSavePath(urDownload, m_pathsSettings->downloadsLocation(), "downloadsLocation", i18n("Downloads"));
    xdgSavePath(urMovie, m_pathsSettings->videosLocation(), "videosLocation", i18n("Movies"));
    xdgSavePath(urPicture, m_pathsSettings->picturesLocation(), "picturesLocation", i18n("Pictures"));
    xdgSavePath(urMusic, m_pathsSettings->musicLocation(), "musicLocation", i18n("Music"));

    m_pathsSettings->save();
}

bool DesktopPathConfig::xdgSavePath(KUrlRequester* ur, const QUrl& currentUrl, const char* xdgKey, const QString& type)
{
    QUrl newUrl = ur->url();
    //url might be empty, use QDir::homePath (the default for xdg) then
    if (!newUrl.isValid()) {
        newUrl = QUrl(QUrl::fromLocalFile(QDir::homePath()));
    }
    if (!newUrl.matches(currentUrl, QUrl::StripTrailingSlash)) {
        const QString path = newUrl.toLocalFile();
        if (!QDir(path).exists()) {
            // Check permissions
            if (QDir().mkpath(path)) {
                QDir().rmdir(path); // rmdir again, so that we get a fast rename
            } else {
                KMessageBox::sorry(this, KIO::buildErrorString(KIO::ERR_CANNOT_MKDIR, path));
                ur->setUrl(currentUrl); // revert
                return false;
            }
        }
        if (moveDir(currentUrl, newUrl, type)) {
            auto item = m_pathsSettings->findItem(xdgKey);
            Q_ASSERT(item);
            item->setProperty(newUrl);
            return true;
        }
    }
    return false;
}

bool DesktopPathConfig::moveDir( const QUrl & src, const QUrl & dest, const QString & type )
{
    if (!src.isLocalFile() || !dest.isLocalFile())
        return true;
    if (!QFile::exists(src.toLocalFile()))
        return true;
    // Do not move $HOME! #193057
    const QString translatedPath = src.toLocalFile();
    if (translatedPath == QDir::homePath() || translatedPath == QDir::homePath() + QLatin1Char('/')) {
        return true;
    }

    m_ok = true;

    QString question;
    KGuiItem yesItem;
    KGuiItem noItem;
    if (QFile::exists(dest.toLocalFile())) {
        // TODO: check if the src dir is empty? Nothing to move, then...
        question = i18n("The path for '%1' has been changed.\nDo you want the files to be moved from '%2' to '%3'?",
                        type, src.toLocalFile(),
                        dest.toLocalFile());
        yesItem = KGuiItem(i18nc("Move files from old to new place", "Move"));
        noItem = KGuiItem(i18nc("Use the new directory but do not move files", "Do not Move"));
    } else {
        question = i18n("The path for '%1' has been changed.\nDo you want to move the directory '%2' to '%3'?",
                        type, src.toLocalFile(),
                        dest.toLocalFile());
        yesItem = KGuiItem(i18nc("Move the directory", "Move"));
        noItem = KGuiItem(i18nc("Use the new directory but do not move anything", "Do not Move"));
    }

    // Ask for confirmation before moving the files
    if (KMessageBox::questionYesNo(this, question, i18n("Confirmation Required"),
                                   yesItem, noItem)
            == KMessageBox::Yes )
    {
        if (QFile::exists(dest.toLocalFile())) {
            // Destination already exists -- should always be the case, for most types,
            // but maybe not for the complex autostart case (to be checked...)
            m_copyToDest = dest;
            m_copyFromSrc = src;
            KIO::ListJob* job = KIO::listDir( src );
            job->setAutoDelete(false); // see <noautodelete> below
            KJobWidgets::setWindow(job, this);
            job->uiDelegate()->setAutoErrorHandlingEnabled(true);
            connect(job, &KIO::ListJob::entries, this, &DesktopPathConfig::slotEntries);
            // slotEntries will move every file/subdir individually into the dest
            job->exec();
            if (m_ok) {
                QDir().rmdir(src.toLocalFile()); // hopefully it's empty by now
            }
            delete job;
        }
        else
        {
            qCDebug(KCM_DESKTOPPATH) << "Direct move from" << src << "to" << dest;
            KIO::Job * job = KIO::move( src, dest );
            KJobWidgets::setWindow(job, this);
            connect(job, &KIO::Job::result, this, &DesktopPathConfig::slotResult);
            job->exec();
        }
    }
    qCDebug(KCM_DESKTOPPATH) << "DesktopPathConfig::slotResult returning " << m_ok;
    return m_ok;
}

void DesktopPathConfig::slotEntries(KIO::Job*, const KIO::UDSEntryList& list)
{
    QListIterator<KIO::UDSEntry> it(list);
    while (it.hasNext()) {
        KFileItem file(it.next(), m_copyFromSrc, true, true);
        qCDebug(KCM_DESKTOPPATH) << file.url();
        if (file.url() == m_copyFromSrc || file.name() == QLatin1String("..")) {
            continue;
        }

        KIO::Job * moveJob = KIO::move(file.url(), m_copyToDest);
        KJobWidgets::setWindow(moveJob, this);
        connect(moveJob, &KIO::Job::result, this, &DesktopPathConfig::slotResult);
        moveJob->exec(); // sub-event loop here. <noautodelete>: the main job is not autodeleted because it would be deleted here
    }
}

void DesktopPathConfig::slotResult( KJob * job )
{
    if (job->error()) {
        if ( job->error() != KIO::ERR_DOES_NOT_EXIST )
            m_ok = false;

        // If the source doesn't exist, no wonder we couldn't move the dir.
        // In that case, trust the user and set the new setting in any case.

        static_cast<KIO::Job*>(job)->uiDelegate()->showErrorMessage();
    }
}

#include "globalpaths.moc"
