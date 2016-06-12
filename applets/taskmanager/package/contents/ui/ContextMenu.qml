/***************************************************************************
 *   Copyright (C) 2012-2016 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaComponents.ContextMenu {
    id: menu

    placement: {
        if (plasmoid.location == PlasmaCore.Types.LeftEdge) {
            return PlasmaCore.Types.RightPosedTopAlignedPopup;
        } else if (plasmoid.location == PlasmaCore.Types.TopEdge) {
            return PlasmaCore.Types.BottomPosedLeftAlignedPopup;
        } else {
            return PlasmaCore.Types.TopPosedLeftAlignedPopup;
        }
    }

    minimumWidth: visualParent.width

    onStatusChanged: {
        if (visualParent && visualParent.launcherUrl != null && status == PlasmaComponents.DialogStatus.Open) {
            launcherToggleAction.checked = (tasksModel.launcherPosition(visualParent.launcherUrl) != -1);
        } else if (status == PlasmaComponents.DialogStatus.Closed) {
            menu.destroy();
        }
    }

    function show() {
        loadDynamicLaunchActions(visualParent.launcherUrl);
        openRelative();
    }

    function newMenuItem(parent) {
        return Qt.createQmlObject(
            "import org.kde.plasma.components 2.0 as PlasmaComponents;" +
            "PlasmaComponents.MenuItem {}",
            parent);
    }

    function newSeparator(parent) {
        return Qt.createQmlObject(
            "import org.kde.plasma.components 2.0 as PlasmaComponents;" +
            "PlasmaComponents.MenuItem { separator: true }",
            parent);
    }

    function loadDynamicLaunchActions(launcherUrl) {
        var actionList = backend.jumpListActions(launcherUrl, menu);

        for (var i = 0; i < actionList.length; ++i) {
            var item = newMenuItem(menu);
            item.action = actionList[i];
            menu.addMenuItem(item, virtualDesktopsMenuItem);
        }

        if (actionList.length > 0) {
            menu.addMenuItem(newSeparator(menu), virtualDesktopsMenuItem);
        }

        var actionList = backend.recentDocumentActions(launcherUrl, menu);

        for (var i = 0; i < actionList.length; ++i) {
            var item = newMenuItem(menu);
            item.action = actionList[i];
            menu.addMenuItem(item, virtualDesktopsMenuItem);
        }

        if (actionList.length > 0) {
            menu.addMenuItem(newSeparator(menu), virtualDesktopsMenuItem);
        }
    }

    PlasmaComponents.MenuItem {
        id: virtualDesktopsMenuItem

        visible: virtualDesktopInfo.numberOfDesktops > 1
            && (visualParent && !visualParent.isLauncher
            && !visualParent.isStartup && visualParent.isVirtualDesktopChangeable)

        enabled: visible

        text: i18n("Move To Desktop")

        Connections {
            target: virtualDesktopInfo

            onNumberOfDesktopsChanged: virtualDesktopsMenu.refresh()
            onDesktopNamesChanged: virtualDesktopsMenu.refresh()
        }

        PlasmaComponents.ContextMenu {
            id: virtualDesktopsMenu

            visualParent: virtualDesktopsMenuItem.action

            function refresh() {
                clearMenuItems();

                if (virtualDesktopInfo.numberOfDesktops <= 1) {
                    return;
                }

                var menuItem = menu.newMenuItem(virtualDesktopsMenu);
                menuItem.text = i18n("Move To Current Desktop");
                menuItem.enabled = Qt.binding(function() {
                    return menu.visualParent && menu.visualParent.virtualDesktop != virtualDesktopInfo.currentDesktop;
                });
                menuItem.clicked.connect(function() {
                    tasksModel.requestVirtualDesktop(menu.visualParent.modelIndex(), 0);
                });

                menuItem = menu.newMenuItem(virtualDesktopsMenu);
                menuItem.text = i18n("All Desktops");
                menuItem.checkable = true;
                menuItem.checked = Qt.binding(function() {
                    return menu.visualParent && menu.visualParent.isOnAllVirtualDesktops;
                });
                menuItem.clicked.connect(function() {
                    tasksModel.requestVirtualDesktop(menu.visualParent.modelIndex(), 0);
                });
                backend.setActionGroup(menuItem.action);

                menu.newSeparator(virtualDesktopsMenu);

                for (var i = 0; i < virtualDesktopInfo.desktopNames.length; ++i) {
                    menuItem = menu.newMenuItem(virtualDesktopsMenu);
                    menuItem.text = i18nc("1 = number of desktop, 2 = desktop name", "%1 Desktop %2", i + 1, virtualDesktopInfo.desktopNames[i]);
                    menuItem.checkable = true;
                    menuItem.checked = Qt.binding((function(i) {
                        return function() { return menu.visualParent && menu.visualParent.virtualDesktop == (i + 1) };
                    })(i));
                    menuItem.clicked.connect((function(i) {
                        return function() { return tasksModel.requestVirtualDesktop(menu.visualParent.modelIndex(), i + 1); };
                    })(i));
                    backend.setActionGroup(menuItem.action);
                }

                menu.newSeparator(virtualDesktopsMenu);

                menuItem = menu.newMenuItem(virtualDesktopsMenu);
                menuItem.text = i18n("New Desktop");
                menuItem.clicked.connect(function() {
                    tasksModel.requestVirtualDesktop(menu.visualParent.modelIndex(), virtualDesktopInfo.numberOfDesktops + 1)
                });
            }

            Component.onCompleted: refresh()
        }
    }

    PlasmaComponents.MenuItem {
        visible: (visualParent && !visualParent.isLauncher && !visualParent.isStartup)

        enabled: visualParent && visualParent.isMinimizable

        checkable: true
        checked: visualParent && visualParent.isMinimized

        text: i18n("Minimize")

        onClicked: tasksModel.requestToggleMinimized(visualParent.modelIndex())
    }

    PlasmaComponents.MenuItem {
        visible: (visualParent && !visualParent.isLauncher && !visualParent.isStartup)

        enabled: visualParent && visualParent.isMaximizable

        checkable: true
        checked: visualParent && visualParent.isMaximized

        text: i18n("Maximize")

        onClicked: tasksModel.requestToggleMaximized(visualParent.modelIndex())
    }

    PlasmaComponents.MenuItem {
        visible: (visualParent && !visualParent.isLauncher && !visualParent.isStartup)

        enabled: visualParent && visualParent.launcherUrl != null

        text: i18n("Start New Instance")
        icon: "system-run"

        onClicked: tasksModel.requestNewInstance(visualParent.modelIndex())
    }

    PlasmaComponents.MenuItem {
        id: launcherToggleAction

        visible: (visualParent && !visualParent.isLauncher && !visualParent.isStartup)

        enabled: visualParent && visualParent.launcherUrl != null

        checkable: true

        text: i18n("Show A Launcher When Not Running")

        onClicked: {
            if (tasksModel.launcherPosition(visualParent.launcherUrl) != -1) {
                tasksModel.requestRemoveLauncher(visualParent.launcherUrl);
            } else {
                tasksModel.requestAddLauncher(visualParent.launcherUrl);
            }
        }
    }

    PlasmaComponents.MenuItem {
        visible: (visualParent && visualParent.isLauncher)

        text: i18n("Remove Launcher")

        onClicked: tasksModel.requestRemoveLauncher(visualParent.launcherUrl);
    }


    PlasmaComponents.MenuItem {
        id: moreActionsMenuItem

        visible: (visualParent && !visualParent.isLauncher && !visualParent.isStartup)

        enabled: visible

        text: i18n("More Actions")

        PlasmaComponents.ContextMenu {
            visualParent: moreActionsMenuItem.action

            PlasmaComponents.MenuItem {
                enabled: menu.visualParent && menu.visualParent.isMovable

                text: i18n("Move")
                icon: "transform-move"

                onClicked: tasksModel.requestMove(menu.visualParent.modelIndex())
            }

            PlasmaComponents.MenuItem {
                enabled: menu.visualParent && menu.visualParent.isResizable

                text: i18n("Resize")

                onClicked: tasksModel.requestResize(menu.visualParent.modelIndex())
            }

            PlasmaComponents.MenuItem {
                checkable: true
                checked: menu.visualParent && menu.visualParent.isKeepAbove

                text: i18n("Keep Above Others")
                icon: "go-up"

                onClicked: tasksModel.requestToggleKeepAbove(menu.visualParent.modelIndex())
            }

            PlasmaComponents.MenuItem {
                checkable: true
                checked: menu.visualParent && menu.visualParent.isKeepBelow

                text: i18n("Keep Below Others")
                icon: "go-down"

                onClicked: tasksModel.requestToggleKeepBelow(menu.visualParent.modelIndex())
            }

            PlasmaComponents.MenuItem {
                enabled: menu.visualParent && menu.visualParent.isFullScreenable

                checkable: true
                checked: menu.visualParent && menu.visualParent.isFullScreen

                text: i18n("Fullscreen")
                icon: "view-fullscreen"

                onClicked: tasksModel.requestToggleFullScreen(menu.visualParent.modelIndex())
            }

            PlasmaComponents.MenuItem {
                enabled: menu.visualParent && menu.visualParent.isShadeable

                checkable: true
                checked: menu.visualParent && menu.visualParent.isShaded

                text: i18n("Shade")

                onClicked: tasksModel.requestToggleShaded(menu.visualParent.modelIndex())
            }

            PlasmaComponents.MenuItem {
                separator: true
            }

            PlasmaComponents.MenuItem {
                visible: (plasmoid.configuration.groupingStrategy != 0) && menu.visualParent.isWindow

                checkable: true
                checked: menu.visualParent && menu.visualParent.isGroupable

                text: i18n("Allow this program to be grouped")

                onClicked: tasksModel.requestToggleGrouping(menu.visualParent.modelIndex())
            }
        }
    }

    PlasmaComponents.MenuItem {
        property QtObject configureAction: null

        enabled: configureAction && configureAction.enabled

        text: configureAction ? configureAction.text : ""
        icon: configureAction ? configureAction.icon : ""

        onClicked: configureAction.trigger()

        Component.onCompleted: configureAction = plasmoid.action("configure")
    }

    PlasmaComponents.MenuItem {
        separator: true
    }

    PlasmaComponents.MenuItem {
        visible: (visualParent && !visualParent.isLauncher && !visualParent.isStartup)

        enabled: visualParent && visualParent.isClosable

        text: i18n("Close")
        icon: "window-close"

        onClicked: tasksModel.requestClose(visualParent.modelIndex())
    }
}
