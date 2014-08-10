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

#ifndef KONQ_COPYTOMENU_H
#define KONQ_COPYTOMENU_H

#include <kurl.h>
#include <kfileitem.h>
#include <libkonq_export.h>

class QMenu;
class KonqCopyToMenuPrivate;

/**
 * This class adds "Copy To" and "Move To" submenus to a popupmenu.
 */
class LIBKONQ_EXPORT KonqCopyToMenu
{
public:
    /**
     * Creates a KonqCopyToMenu instance.
     * Note that this instance must stay alive for at least as long as the popupmenu;
     * it has the slots for the actions created by addActionsTo.
     * @deprecated
     * KDE5: remove, so that passing a parent widget is mandatory
     */
    KonqCopyToMenu();

    /**
     * Creates a KonqCopyToMenu instance, with a parent widget for the file dialog
     * and message boxes.
     * Note that this instance (and the widget) must stay alive for at least as
     * long as the popupmenu; it has the slots for the actions created by addActionsTo.
     * @param widget note that this is not the parent of KonqCopyToMenu itself.
     * @since 4.2
     */
    KonqCopyToMenu(QWidget* parentWidget);

    /**
     * Destructor
     */
    ~KonqCopyToMenu();

    /**
     * Sets the list of fileitems which the actions apply to.
     * Either call this or setUrls.
     */
    void setItems(const KFileItemList& items);

    /**
     * Sets the URLs which the actions apply to.
     * Either call this or setItems.
     */
    void setUrls(const KUrl::List& urls);

    /**
     * If setReadOnly(true) is called, the "Move To" submenu will not appear.
     */
    void setReadOnly(bool ro);

    /**
     * Generate the actions and submenus, and adds them to the @p menu.
     * All actions are created as children of the menu.
     */
    void addActionsTo(QMenu* menu);

private:
    KonqCopyToMenuPrivate* const d;
};

#endif /* KONQ_COPYTOMENU_H */
