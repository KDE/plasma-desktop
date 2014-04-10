/* This file is part of the KDE project

   Copyright 2008 David Faure <faure@kde.org>

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

#include <kconfiggroup.h>
#include <kmenu.h>
#include <QActionGroup>
#include <QObject>
#include <kurl.h>

class KonqCopyToMenuPrivate
{
public:
    KonqCopyToMenuPrivate(QWidget* parentWidget = 0);

    KUrl::List m_urls;
    bool m_readOnly;
    QWidget* m_parentWidget;
};

enum MenuType { Copy, Move };

// The main menu, shown when opening "Copy To" or "Move To"
// It contains Home Folder, Root Folder, Browse, and recent destinations
class KonqCopyToMainMenu : public KMenu
{
    Q_OBJECT
public:
    KonqCopyToMainMenu(QMenu* parent, KonqCopyToMenuPrivate* d, MenuType menuType);

    QActionGroup& actionGroup() { return m_actionGroup; } // used by submenus
    MenuType menuType() const { return m_menuType; } // used by submenus

private Q_SLOTS:
    void slotAboutToShow();
    void slotBrowse();
    void slotTriggered(QAction* action);

private:
    void copyOrMoveTo(const KUrl& dest);

private:
    MenuType m_menuType;
    QActionGroup m_actionGroup;
    KonqCopyToMenuPrivate* d; // this isn't our own d pointer, it's the one for the public class
    KConfigGroup m_recentDirsGroup;
};

// The menu that lists a directory
class KonqCopyToDirectoryMenu : public KMenu
{
    Q_OBJECT
public:
    KonqCopyToDirectoryMenu(QMenu* parent, KonqCopyToMainMenu* mainMenu, const QString& path);

private Q_SLOTS:
    void slotAboutToShow();

private:
    KonqCopyToMainMenu* m_mainMenu;
    QString m_path;
};
