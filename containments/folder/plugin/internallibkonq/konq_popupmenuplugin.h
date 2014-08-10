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

#ifndef KONQ_POPUPMENUPLUGIN_H
#define KONQ_POPUPMENUPLUGIN_H

#include <QObject>

class QMenu;
class KActionCollection;
class KFileItemListProperties;

/**
 * Base class for KonqPopupMenu plugins.
 *
 * Please try to use servicemenus first, if you simply need to add
 * actions to the popup menu for one or more mimetypes.
 *
 * However if you need some dynamic logic, like "only show this item if
 * two files are selected", or "show a submenu with a variable number of actions",
 * then you have to implement a KonqPopupMenuPlugin subclass.
 *
 * @deprecated Use KFileItemActionPlugin instead.
 */
class KonqPopupMenuPlugin : public QObject
{
    Q_OBJECT
public:

    /**
     * Constructor.
     */
    KonqPopupMenuPlugin(QObject* parent);
    virtual ~KonqPopupMenuPlugin();

    /**
     * Implement the setup method in the plugin in order to create actions
     * in the given actionCollection and add it to the menu using menu->addAction().
     *
     * @param actionCollection the parent for the actions
     * @param popupMenuInfo all the information about the popupmenu being shown
     * (which file items, their common mimetype, etc.)
     * @param menu the menu where the plugin can add its own actions
     */
    virtual void setup(KActionCollection* actionCollection,
                       const KFileItemListProperties& popupMenuInfo,
                       QMenu *menu) = 0;
};

#endif /* KONQ_POPUPMENUPLUGIN_H */
