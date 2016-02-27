/* This file is part of the KDE project
   Copyright (C) 1998-2016 David Faure <faure@kde.org>
   Copyright (C) 2001 Holger Freyther <freyther@yahoo.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __konqpopupmenu_h
#define __konqpopupmenu_h

#include <QMenu>

#include <libkonq_export.h>

class KFileItemList;
class KNewFileMenu;
class KFileItemActions;

class KActionCollection;
class KBookmarkManager;
class KonqPopupMenuPrivate;
class QUrl;

/**
 * This class implements the popup menu for URLs in file managers.
 *
 * The recommended usage is to use QMenu::popup() to avoid modal popupmenus.
 *
 * Either reuse the instance, or delete it after use with
 *   connect(menu, &QMenu::aboutToHide, [menu]() { menu->deleteLater(); });
 *
 * Users of KonqPopupMenu include: konqueror, kfind, the folderview desktop plasmoid.
 */
class LIBKONQ_EXPORT KonqPopupMenu : public QMenu
{
    Q_OBJECT
public:

    /**
     * Each action group is inserted into the context menu at a specific position.
     * "editactions" for actions related text editing,
     * "linkactions" for actions related to hyperlinks,
     * "partactions" for any other actions provided by the part
     */
    enum ActionGroup {
        TopActions,  ///< actions to be shown at the top of the context menu, like "show menu bar"
        TabHandlingActions, ///< actions for tab handling, like "open in new tab"
        EditActions, ///< move to trash, delete, etc.
        PreviewActions, ///< actions related to viewing the selected file with an embedded viewer
        CustomActions, ///< group for more custom actions, such as those provided by KParts components
        LinkActions  ///< actions related to handling of hyperlinks, only shown if the IsLink flag is set
    };

    typedef QMap<ActionGroup, QList<QAction *> > ActionGroupMap;

    /**
     * Set of flags to ask for some items in the popup menu.
     */
    enum Flag {
        DefaultPopupItems = 0x0000, ///< default value, no additional menu item
        ShowNavigationItems = 0x0001, ///< show "back" and "forward" (usually done when clicking the background of the view, but not an item)
        ShowUp = 0x0002, ///<  show "up" (same thing, but not over e.g. HTTP). Requires ShowNavigationItems.
        ShowReload = 0x0004, ///< show "reload" (usually done when clicking the background of the view, but not an item)
        ShowBookmark = 0x0008, ///< show "add to bookmarks" (usually not done on the local filesystem)
        ShowCreateDirectory = 0x0010, ///<  show "create directory" (usually only done on the background of the view, or
                                      /// in hierarchical views like directory trees, where the new dir would be visible)
        ShowTextSelectionItems = 0x0020, ///< set when selecting text, for a popup that only contains text-related items.
        NoDeletion = 0x0040, ///< "cut" not allowed (e.g. parent dir not writeable).
                             /// (this is only needed if the protocol itself supports deletion, unlike e.g. HTTP)
        IsLink = 0x0080, ///< show "Bookmark This Link" and other link-related actions (LinkActions group)
        ShowUrlOperations = 0x0100, ///< show copy, paste, as well as cut if NoDeletion is not set.
        ShowProperties = 0x0200,   ///< show "Properties" action (usually done by directory views)
        ShowNewWindow = 0x0400, ///< Show "Open in new window" action
        NoPlugins = 0x0800      ///< Disable plugins - mostly for the unittest
     };
    Q_DECLARE_FLAGS(Flags, Flag)

    /**
     * Constructor
     * @param items the list of file items the popupmenu should be shown for
     * @param viewURL the URL shown in the view, to test for RMB click on view background
     * @param actions list of actions the caller wants to see in the menu
     * @param newMenu "New" menu, shared with the File menu, in konqueror
     * @param flags flags which control which items to show in the popupmenu
     * @param parentWidget the widget we're showing this popup for. Helps destroying
     * the popup if the widget is destroyed before the popup.
     *
     * The actions to pass in the action collection are :
     * go_back, go_forward, go_up, reload, cut, copy, paste, pasteto
     * The others items are automatically inserted.
     */
    KonqPopupMenu(const KFileItemList &items,
                  const QUrl &viewURL,
                  KActionCollection &actions,
                  Flags flags,
                  QWidget *parentWidget = 0);

    /**
     * Don't forget to destroy the object
     */
    ~KonqPopupMenu();

    /**
     * Sets the "New file" menu instance. This allows to share it with the menubar.
     */
    void setNewFileMenu(KNewFileMenu *newMenu);

    /**
     * Sets the bookmark manager for the "add to bookmark" action.
     * Only used if ShowBookmark is set
     */
    void setBookmarkManager(KBookmarkManager *manager);

    /**
     * Sets the additional actions to show in the context menu
     */
    void setActionGroups(const ActionGroupMap &actionGroups);

    /**
     * Set the title of the URL, when the popupmenu is opened for a single URL.
     * This is used if the user chooses to add a bookmark for this URL.
     */
    void setURLTitle(const QString &urlTitle);

Q_SIGNALS:
    /**
     * Emitted before the "Open With" dialog is shown
     * This is used e.g in folderview to close the folder peek popups on invoking the "Open With" menu action
     */
    void openWithDialogAboutToBeShown();

private:
    KonqPopupMenuPrivate *d;
};

#endif
