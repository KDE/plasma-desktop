/*  This file is part of the KDE project
    Copyright 2009  Harald Hvaal <haraldhv@stud.ntnu.no>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef _KONQ_DNDPOPUPMENUPLUGIN_H_
#define _KONQ_DNDPOPUPMENUPLUGIN_H_

#include <QObject>

class QMenu;
class KActionCollection;
class KFileItemListProperties;
class QUrl;
class QAction;

/**
 * Base class for drag and drop popup menus
 *
 * This can be used for adding dynamic menu items to the normal copy/move/link
 * here menu appearing in dolphin/konqueror. In the setup-method you may check
 * the properties of the dropped files, and if applicable, append your own
 * QAction that the user may trigger in the menu.
 *
 * @author Harald Hvaal <metellius@gmail.com>
 */
class KonqDndPopupMenuPlugin : public QObject
{
    Q_OBJECT
public:

    /**
     * Constructor.
     */
    KonqDndPopupMenuPlugin(QObject* parent);
    virtual ~KonqDndPopupMenuPlugin();

    /**
     * Implement the setup method in the plugin in order to create actions
     * in the given actionCollection and add it to the menu using menu->addAction().
     *
     * @param popupMenuInfo all the information about the popupmenu being shown
     * (which file items, their common mimetype, etc.)
     * @param destination the URL to where the file(s) were dropped
     * @param pluginActions a QList with the QActions that will be plugged into
     * the menu.
     */
    virtual void setup(const KFileItemListProperties& popupMenuInfo,
            QUrl destination,
            QList<QAction*>& pluginActions) = 0;
};

#endif /* _KONQ_DNDPOPUPMENUPLUGIN_H_ */
