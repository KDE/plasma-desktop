/* This file is part of the KDE project

   Copyright 2008, 2009 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "konq_copytomenu.h"
#include "konq_copytomenu_p.h"
#include <QAction>
#include <kdebug.h>
#include <QIcon>
#include <kglobal.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <kstringhandler.h>
#include <KJobWidgets>
#include <KIO/FileUndoManager>
#include <KIO/CopyJob>
#include <KIO/JobUiDelegate>
#include <QDir>

#ifdef Q_OS_WIN
#include "Windows.h"
#endif

KonqCopyToMenuPrivate::KonqCopyToMenuPrivate(QWidget* parentWidget)
    : m_urls(), m_readOnly(false), m_parentWidget(parentWidget)
{
}

////

KonqCopyToMenu::KonqCopyToMenu(QWidget* parentWidget)
    : d(new KonqCopyToMenuPrivate(parentWidget))
{
}

KonqCopyToMenu::~KonqCopyToMenu()
{
    delete d;
}

void KonqCopyToMenu::setItems(const KFileItemList& items)
{
    // For now we lose all the information except for the urls
    // But this API is useful in case KIO can make use of this information later
    // (e.g. to avoid stat'ing the source urls)
    Q_FOREACH(const KFileItem& item, items)
        d->m_urls.append(item.url());
}

void KonqCopyToMenu::setUrls(const QList<QUrl> &urls)
{
    d->m_urls = urls;
}

void KonqCopyToMenu::setReadOnly(bool ro)
{
    d->m_readOnly = ro;
}

void KonqCopyToMenu::addActionsTo(QMenu* menu)
{
    KMenu* mainCopyMenu = new KonqCopyToMainMenu(menu, d, Copy);
    mainCopyMenu->setTitle(i18nc("@title:menu", "Copy To"));
    mainCopyMenu->menuAction()->setObjectName( QLatin1String("copyTo_submenu" )); // for the unittest
    menu->addMenu(mainCopyMenu);

    if (!d->m_readOnly) {
        KMenu* mainMoveMenu = new KonqCopyToMainMenu(menu, d, Move);
        mainMoveMenu->setTitle(i18nc("@title:menu", "Move To"));
        mainMoveMenu->menuAction()->setObjectName( QLatin1String("moveTo_submenu" )); // for the unittest
        menu->addMenu(mainMoveMenu);
    }
}

////

KonqCopyToMainMenu::KonqCopyToMainMenu(QMenu* parent, KonqCopyToMenuPrivate* _d, MenuType menuType)
    : KMenu(parent), m_menuType(menuType),
      m_actionGroup(static_cast<QWidget *>(0)),
      d(_d),
      m_recentDirsGroup(KSharedConfig::openConfig(), m_menuType == Copy ? "kuick-copy" : "kuick-move")
{
    connect(this, &KonqCopyToMainMenu::aboutToShow, this, &KonqCopyToMainMenu::slotAboutToShow);
    connect(&m_actionGroup, &QActionGroup::triggered, this, &KonqCopyToMainMenu::slotTriggered);
}

void KonqCopyToMainMenu::slotAboutToShow()
{
    clear();
    KonqCopyToDirectoryMenu* subMenu;
    // Home Folder
    subMenu = new KonqCopyToDirectoryMenu(this, this, QDir::homePath());
    subMenu->setTitle(i18nc("@title:menu", "Home Folder"));
    subMenu->setIcon(QIcon::fromTheme("go-home"));
    addMenu(subMenu);

    // Root Folder
#ifndef Q_OS_WIN
    subMenu = new KonqCopyToDirectoryMenu(this, this, QDir::rootPath());
    subMenu->setTitle(i18nc("@title:menu", "Root Folder"));
    subMenu->setIcon(QIcon::fromTheme("folder-red"));
    addMenu(subMenu);
#else
    foreach ( const QFileInfo& info, QDir::drives() ) {
        uint type = DRIVE_UNKNOWN;
        QString driveIcon = "drive-harddisk";
        QT_WA({ type = GetDriveTypeW((wchar_t *)info.absoluteFilePath().utf16()); },
              { type = GetDriveTypeA(info.absoluteFilePath().toLocal8Bit()); });
        switch (type) {
            case DRIVE_REMOVABLE:
                driveIcon = "drive-removable-media";
                break;
            case DRIVE_FIXED:
                driveIcon = "drive-harddisk";
                break;
            case DRIVE_REMOTE:
                driveIcon = "network-server";
                break;
            case DRIVE_CDROM:
                driveIcon = "drive-optical";
                break;
            case DRIVE_RAMDISK:
            case DRIVE_UNKNOWN:
            case DRIVE_NO_ROOT_DIR:
            default:
                driveIcon = "drive-harddisk";
        }
        subMenu = new KonqCopyToDirectoryMenu(this, this, info.absoluteFilePath());
        subMenu->setTitle(info.absoluteFilePath());
        subMenu->setIcon(QIcon::fromTheme(driveIcon));
        addMenu(subMenu);
    }
#endif

    // Browse... action, shows a KFileDialog
    QAction * browseAction = new QAction(i18nc("@title:menu in Copy To or Move To submenu", "Browse..."), this);
    connect(browseAction, &QAction::triggered, this, &KonqCopyToMainMenu::slotBrowse);
    addAction(browseAction);

    addSeparator(); // looks like Qt4 handles removing it automatically if it's last in the menu, nice.

    // Recent Destinations
    const QStringList recentDirs = m_recentDirsGroup.readPathEntry("Paths", QStringList());
    Q_FOREACH(const QString& recentDir, recentDirs) {
        const QUrl url(recentDir);
        const QString text = KStringHandler::csqueeze(url.toDisplayString(), 60); // shorten very long paths (#61386)
        QAction * act = new QAction(text, this);
        act->setData(url);
        m_actionGroup.addAction(act);
        addAction(act);
    }
}

void KonqCopyToMainMenu::slotBrowse()
{
    const QUrl dest = KFileDialog::getExistingDirectoryUrl(QUrl("kfiledialog:///copyto"), // FIXME
                                                           d->m_parentWidget ? d->m_parentWidget : this);
    if (!dest.isEmpty()) {
        copyOrMoveTo(dest);
    }
}

void KonqCopyToMainMenu::slotTriggered(QAction* action)
{
    const QUrl url = action->data().value<QUrl>();
    Q_ASSERT(!url.isEmpty());
    copyOrMoveTo(url);
}

void KonqCopyToMainMenu::copyOrMoveTo(const QUrl& dest)
{
    // Insert into the recent destinations list
    QStringList recentDirs = m_recentDirsGroup.readPathEntry("Paths", QStringList());
    const QString niceDest = dest.toDisplayString();
    if (!recentDirs.contains(niceDest)) { // don't change position if already there, moving stuff is bad usability
        recentDirs.prepend(niceDest);
        while (recentDirs.size() > 10) { // hardcoded max size
            recentDirs.removeLast();
        }
        m_recentDirsGroup.writePathEntry("Paths", recentDirs);
    }

    // #199549: add a trailing slash to avoid unexpected results when the
    // dest doesn't exist anymore: it was creating a file with the name of
    // the now non-existing dest.
    QUrl dirDest = dest;
    if (!dirDest.path().endsWith('/'))
        dirDest.setPath(dirDest.path() + '/');

    // And now let's do the copy or move -- with undo/redo support.
    KIO::CopyJob* job = m_menuType == Copy ? KIO::copy(d->m_urls, dirDest) : KIO::move(d->m_urls, dirDest);
    KIO::FileUndoManager::self()->recordCopyJob(job);
    KJobWidgets::setWindow(job, d->m_parentWidget ? d->m_parentWidget : this);
    job->ui()->setAutoErrorHandlingEnabled(true); // or connect to the result signal
}

////

KonqCopyToDirectoryMenu::KonqCopyToDirectoryMenu(QMenu* parent, KonqCopyToMainMenu* mainMenu, const QString& path)
    : KMenu(parent), m_mainMenu(mainMenu), m_path(path)
{
    connect(this, &KonqCopyToDirectoryMenu::aboutToShow, this, &KonqCopyToDirectoryMenu::slotAboutToShow);
}

void KonqCopyToDirectoryMenu::slotAboutToShow()
{
    clear();
    QAction * act = new QAction(m_mainMenu->menuType() == Copy
                               ? i18nc("@title:menu", "Copy Here")
                               : i18nc("@title:menu", "Move Here"), this);
    act->setData(QUrl::fromLocalFile(m_path));
    act->setEnabled(QFileInfo(m_path).isWritable());
    m_mainMenu->actionGroup().addAction(act);
    addAction(act);

    addSeparator(); // looks like Qt4 handles removing it automatically if it's last in the menu, nice.

    // List directory
    // All we need is sub folder names, their permissions, their icon.
    // KDirLister or KIO::listDir would fetch much more info, and would be async,
    // and we only care about local directories so we use QDir directly.
    QDir dir(m_path);
    const QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::LocaleAware);
    KMimeType::Ptr dirMime = KMimeType::mimeType("inode/directory");
    Q_FOREACH(const QString& subDir, entries) {
        QString subPath = m_path;
        if (!subPath.endsWith('/'))
            subPath.append('/');
        subPath += subDir;
        KonqCopyToDirectoryMenu* subMenu = new KonqCopyToDirectoryMenu(this, m_mainMenu, subPath);
        QString menuTitle(subDir);
        // Replace '&' by "&&" to make sure that '&' inside the directory name is displayed
        // correctly and not misinterpreted as an indicator for a keyboard shortcut
        subMenu->setTitle(menuTitle.replace('&', "&&"));
        const QString iconName = dirMime->iconName();
        subMenu->setIcon(QIcon::fromTheme(iconName));
        if (QFileInfo(subPath).isSymLink()) { // I hope this isn't too slow...
            QFont font = subMenu->menuAction()->font();
            font.setItalic(true);
            subMenu->menuAction()->setFont(font);
        }
        addMenu(subMenu);
    }
}
