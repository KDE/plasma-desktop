/***************************************************************************
 *   Copyright (C) 2012-2016 by Eike Hein <hein@kde.org>                   *
 *   Copyright (C) 2016 by Kai Uwe Broulik <kde@privat.broulik.de>         *
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

import org.kde.taskmanager 0.1 as TaskManager

import "code/layout.js" as LayoutManager

PlasmaComponents.ContextMenu {
    id: menu

    property QtObject backend
    property QtObject mpris2Source
    property var modelIndex
    readonly property var atm: TaskManager.AbstractTasksModel

    property bool showAllPlaces: false

    placement: {
        if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
            return PlasmaCore.Types.RightPosedTopAlignedPopup;
        } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
            return PlasmaCore.Types.BottomPosedLeftAlignedPopup;
        } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
            return PlasmaCore.Types.LeftPosedTopAlignedPopup;
        } else {
            return PlasmaCore.Types.TopPosedLeftAlignedPopup;
        }
    }

    minimumWidth: visualParent.width

    onStatusChanged: {
        if (visualParent && get(atm.LauncherUrlWithoutIcon) != "" && status == PlasmaComponents.DialogStatus.Open) {
            activitiesDesktopsMenu.refresh();

        } else if (status == PlasmaComponents.DialogStatus.Closed) {
            menu.destroy();
            backend.ungrabMouse(visualParent);
        }
    }

    Component.onCompleted: {
        // Cannot have "Connections" as child of PlasmaCoponents.ContextMenu.
        backend.showAllPlaces.connect(function() {
            visualParent.showContextMenu({showAllPlaces: true});
        });
    }

    function get(modelProp) {
        return tasksModel.data(modelIndex, modelProp)
    }

    function show() {
        loadDynamicLaunchActions(get(atm.LauncherUrlWithoutIcon));
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
        var sections = [
            {
                title:   i18n("Places"),
                group:   "places",
                actions: backend.placesActions(launcherUrl, showAllPlaces, menu)
            },
            {
                title:   i18n("Recent Files"),
                group:   "recents",
                actions: backend.recentDocumentActions(launcherUrl, menu)
            },
            {
                title:   i18n("Actions"),
                group:   "actions",
                actions: backend.jumpListActions(launcherUrl, menu)
            }
        ]

        // QMenu does not limit its width automatically. Even if we set a maximumWidth
        // it would just cut off text rather than eliding. So we do this manually.
        var textMetrics = Qt.createQmlObject("import QtQuick 2.4; TextMetrics {}", menu);
        var maximumWidth = LayoutManager.maximumContextMenuTextWidth();

        sections.forEach(function (section) {
            if (section["actions"].length > 0 || section["group"] == "actions") {
                // Don't add the "Actions" header if the menu has nothing but actions
                // in it, because then it's redundant (all menus have actions)
                if (
                    (section["group"] != "actions") ||
                    (section["group"] == "actions" && (sections[0]["actions"].length > 0 || sections[1]["actions"].length > 0))
                ) {
                    var sectionHeader = newMenuItem(menu);
                    sectionHeader.text = section["title"];
                    sectionHeader.section = true;
                    menu.addMenuItem(sectionHeader, startNewInstanceItem);
                }
            }

            for (var i = 0; i < section["actions"].length; ++i) {
                var item = newMenuItem(menu);
                item.action = section["actions"][i];

                // Crude way of manually eliding...
                var elided = false;
                textMetrics.text = Qt.binding(function() {
                    return item.action.text;
                });

                while (textMetrics.width > maximumWidth) {
                    item.action.text = item.action.text.slice(0, -1);
                    elided = true;
                }

                if (elided) {
                    item.action.text += "…";
                }

                menu.addMenuItem(item, startNewInstanceItem);
            }
        });

        // Add Media Player control actions
        var sourceName = mpris2Source.sourceNameForLauncherUrl(launcherUrl, get(atm.AppPid));

        if (sourceName && !(get(atm.WinIdList) !== undefined && get(atm.WinIdList).length > 1)) {
            var playerData = mpris2Source.data[sourceName]

            if (playerData.CanControl) {
                var playing = (playerData.PlaybackStatus === "Playing");
                var menuItem = menu.newMenuItem(menu);
                menuItem.text = i18nc("Play previous track", "Previous Track");
                menuItem.icon = "media-skip-backward";
                menuItem.enabled = Qt.binding(function() {
                    return playerData.CanGoPrevious;
                });
                menuItem.clicked.connect(function() {
                    mpris2Source.goPrevious(sourceName);
                });
                menu.addMenuItem(menuItem, startNewInstanceItem);

                menuItem = menu.newMenuItem(menu);
                // PlasmaCore Menu doesn't actually handle icons or labels changing at runtime...
                menuItem.text = Qt.binding(function() {
                    // if CanPause, toggle the menu entry between Play & Pause, otherwise always use Play
                    return playing && playerData.CanPause ? i18nc("Pause playback", "Pause") : i18nc("Start playback", "Play");
                });
                menuItem.icon = Qt.binding(function() {
                    return playing && playerData.CanPause ? "media-playback-pause" : "media-playback-start";
                });
                menuItem.enabled = Qt.binding(function() {
                    return playing ? playerData.CanPause : playerData.CanPlay;
                });
                menuItem.clicked.connect(function() {
                    if (playing) {
                        mpris2Source.pause(sourceName);
                    } else {
                        mpris2Source.play(sourceName);
                    }
                });
                menu.addMenuItem(menuItem, startNewInstanceItem);

                menuItem = menu.newMenuItem(menu);
                menuItem.text = i18nc("Play next track", "Next Track");
                menuItem.icon = "media-skip-forward";
                menuItem.enabled = Qt.binding(function() {
                    return playerData.CanGoNext;
                });
                menuItem.clicked.connect(function() {
                    mpris2Source.goNext(sourceName);
                });
                menu.addMenuItem(menuItem, startNewInstanceItem);

                menuItem = menu.newMenuItem(menu);
                menuItem.text = i18nc("Stop playback", "Stop");
                menuItem.icon = "media-playback-stop";
                menuItem.enabled = Qt.binding(function() {
                    return playerData.PlaybackStatus !== "Stopped";
                });
                menuItem.clicked.connect(function() {
                    mpris2Source.stop(sourceName);
                });
                menu.addMenuItem(menuItem, startNewInstanceItem);

                // Technically media controls and audio streams are separate but for the user they're
                // semantically related, don't add a separator inbetween.
                if (!menu.visualParent.hasAudioStream) {
                    menu.addMenuItem(newSeparator(menu), startNewInstanceItem);
                }

                // If we don't have a window associated with the player but we can quit
                // it through MPRIS we'll offer a "Quit" option instead of "Close"
                if (!closeWindowItem.visible && playerData.CanQuit) {
                    menuItem = menu.newMenuItem(menu);
                    menuItem.text = i18nc("Quit media player app", "Quit");
                    menuItem.icon = "application-exit";
                    menuItem.visible = Qt.binding(function() {
                        return !closeWindowItem.visible;
                    });
                    menuItem.clicked.connect(function() {
                        mpris2Source.quit(sourceName);
                    });
                    menu.addMenuItem(menuItem);
                }

                // If we don't have a window associated with the player but we can raise
                // it through MPRIS we'll offer a "Restore" option
                if (!startNewInstanceItem.visible && playerData.CanRaise) {
                    menuItem = menu.newMenuItem(menu);
                    menuItem.text = i18nc("Open or bring to the front window of media player app", "Restore");
                    menuItem.icon = playerData["Desktop Icon Name"];
                    menuItem.visible = Qt.binding(function() {
                        return !startNewInstanceItem.visible;
                    });
                    menuItem.clicked.connect(function() {
                        mpris2Source.raise(sourceName);
                    });
                    menu.addMenuItem(menuItem, startNewInstanceItem);
                }
            }
        }

        // We allow mute/unmute whenever an application has a stream, regardless of whether it
        // is actually playing sound.
        // This way you can unmute, e.g. a telephony app, even after the conversation has ended,
        // so you still have it ringing later on.
        if (menu.visualParent.hasAudioStream) {
            var muteItem = menu.newMenuItem(menu);
            muteItem.checkable = true;
            muteItem.checked = Qt.binding(function() {
                return menu.visualParent && menu.visualParent.muted;
            });
            muteItem.clicked.connect(function() {
                menu.visualParent.toggleMuted();
            });
            muteItem.text = i18n("Mute");
            muteItem.icon = "audio-volume-muted";
            menu.addMenuItem(muteItem, startNewInstanceItem);

            menu.addMenuItem(newSeparator(menu), startNewInstanceItem);
        }
    }

    PlasmaComponents.MenuItem {
        id: startNewInstanceItem
        visible: (visualParent && get(atm.IsLauncher) !== true && get(atm.IsStartup) !== true)

        enabled: visualParent && get(atm.LauncherUrlWithoutIcon) != ""

        text: i18n("Start New Instance")
        icon: "list-add-symbolic"

        onClicked: tasksModel.requestNewInstance(modelIndex)
    }

        PlasmaComponents.MenuItem {
        id: virtualDesktopsMenuItem

        visible: virtualDesktopInfo.numberOfDesktops > 1
            && (visualParent && get(atm.IsLauncher) !== true
            && get(atm.IsStartup) !== true
            && get(atm.IsVirtualDesktopsChangeable) === true)

        enabled: visible

        text: i18n("Move to &Desktop")
        icon: "virtual-desktops"

        Connections {
            target: virtualDesktopInfo

            function onNumberOfDesktopsChanged() {Qt.callLater(virtualDesktopsMenu.refresh)}
            function onDesktopIdsChanged() {Qt.callLater(virtualDesktopsMenu.refresh)}
            function onDesktopNamesChanged() {Qt.callLater(virtualDesktopsMenu.refresh)}
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
                menuItem.text = i18n("Move &To Current Desktop");
                menuItem.enabled = Qt.binding(function() {
                    return menu.visualParent && menu.get(atm.VirtualDesktops).indexOf(virtualDesktopInfo.currentDesktop) === -1;
                });
                menuItem.clicked.connect(function() {
                    tasksModel.requestVirtualDesktops(menu.modelIndex, [virtualDesktopInfo.currentDesktop]);
                });

                menuItem = menu.newMenuItem(virtualDesktopsMenu);
                menuItem.text = i18n("&All Desktops");
                menuItem.checkable = true;
                menuItem.checked = Qt.binding(function() {
                    return menu.visualParent && menu.get(atm.IsOnAllVirtualDesktops) === true;
                });
                menuItem.clicked.connect(function() {
                    tasksModel.requestVirtualDesktops(menu.modelIndex, []);
                });
                backend.setActionGroup(menuItem.action);

                menu.newSeparator(virtualDesktopsMenu);

                for (var i = 0; i < virtualDesktopInfo.desktopNames.length; ++i) {
                    menuItem = menu.newMenuItem(virtualDesktopsMenu);
                    menuItem.text = i18nc("1 = number of desktop, 2 = desktop name", "&%1 %2", i + 1, virtualDesktopInfo.desktopNames[i]);
                    menuItem.checkable = true;
                    menuItem.checked = Qt.binding((function(i) {
                        return function() { return menu.visualParent && menu.get(atm.VirtualDesktops).indexOf(virtualDesktopInfo.desktopIds[i]) > -1 };
                    })(i));
                    menuItem.clicked.connect((function(i) {
                        return function() { return tasksModel.requestVirtualDesktops(menu.modelIndex, [virtualDesktopInfo.desktopIds[i]]); };
                    })(i));
                    backend.setActionGroup(menuItem.action);
                }

                menu.newSeparator(virtualDesktopsMenu);

                menuItem = menu.newMenuItem(virtualDesktopsMenu);
                menuItem.text = i18n("&New Desktop");
                menuItem.clicked.connect(function() {
                    tasksModel.requestNewVirtualDesktop(menu.modelIndex);
                });
            }

            Component.onCompleted: refresh()
        }
    }

     PlasmaComponents.MenuItem {
        id: activitiesDesktopsMenuItem

        visible: activityInfo.numberOfRunningActivities > 1
            && (visualParent && !get(atm.IsLauncher)
            && !get(atm.IsStartup))

        enabled: visible

        text: i18n("Show in &Activities")
        icon: "activities"

        Connections {
            target: activityInfo

            function onNumberOfRunningActivitiesChanged() {
                activitiesDesktopsMenu.refresh()
            }
        }

        PlasmaComponents.ContextMenu {
            id: activitiesDesktopsMenu

            visualParent: activitiesDesktopsMenuItem.action

            function refresh() {
                clearMenuItems();

                if (activityInfo.numberOfRunningActivities <= 1) {
                    return;
                }

                var menuItem = menu.newMenuItem(activitiesDesktopsMenu);
                menuItem.text = i18n("Add To Current Activity");
                menuItem.enabled = Qt.binding(function() {
                    return menu.visualParent && menu.get(atm.Activities).length > 0 &&
                           menu.get(atm.Activities).indexOf(activityInfo.currentActivity) < 0;
                });
                menuItem.clicked.connect(function() {
                    tasksModel.requestActivities(menu.modelIndex, menu.get(atm.Activities).concat(activityInfo.currentActivity));
                });

                menuItem = menu.newMenuItem(activitiesDesktopsMenu);
                menuItem.text = i18n("All Activities");
                menuItem.checkable = true;
                menuItem.checked = Qt.binding(function() {
                    return menu.visualParent && menu.get(atm.Activities).length === 0;
                });
                menuItem.toggled.connect(function(checked) {
                    var newActivities = undefined; // will cast to an empty QStringList i.e all activities
                    if (!checked) {
                        newActivities = new Array(activityInfo.currentActivity);
                    }
                    tasksModel.requestActivities(menu.modelIndex, newActivities);
                });

                menu.newSeparator(activitiesDesktopsMenu);

                var runningActivities = activityInfo.runningActivities();
                for (var i = 0; i < runningActivities.length; ++i) {
                    var activityId = runningActivities[i];

                    menuItem = menu.newMenuItem(activitiesDesktopsMenu);
                    menuItem.text = activityInfo.activityName(runningActivities[i]);
                    menuItem.checkable = true;
                    menuItem.checked = Qt.binding( (function(activityId) {
                        return function() {
                            return menu.visualParent && menu.get(atm.Activities).indexOf(activityId) >= 0;
                        };
                    })(activityId));
                    menuItem.toggled.connect((function(activityId) {
                        return function (checked) {
                            var newActivities = menu.get(atm.Activities);
                            if (checked) {
                                newActivities = newActivities.concat(activityId);
                            } else {
                                var index = newActivities.indexOf(activityId)
                                if (index < 0) {
                                    return;
                                }

                                newActivities.splice(index, 1);
                            }
                            return tasksModel.requestActivities(menu.modelIndex, newActivities);
                        };
                    })(activityId));
                }

                menu.newSeparator(activitiesDesktopsMenu);
            }

            Component.onCompleted: refresh()
        }
    }

    PlasmaComponents.MenuItem {
        id: moreActionsMenuItem

        visible: (visualParent && get(atm.IsLauncher) !== true && get(atm.IsStartup) !== true)

        enabled: visible

        text: i18n("More Actions")
        icon: "view-more-symbolic"

        PlasmaComponents.ContextMenu {
            visualParent: moreActionsMenuItem.action

            PlasmaComponents.MenuItem {
                enabled: menu.visualParent && menu.get(atm.IsMovable) === true

                text: i18n("&Move")
                icon: "transform-move"

                onClicked: tasksModel.requestMove(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                enabled: menu.visualParent && menu.get(atm.IsResizable) === true

                text: i18n("Re&size")
                icon: "transform-scale"

                onClicked: tasksModel.requestResize(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                visible: (menu.visualParent && get(atm.IsLauncher) !== true && get(atm.IsStartup) !== true)

                enabled: menu.visualParent && get(atm.IsMaximizable) === true

                checkable: true
                checked: menu.visualParent && get(atm.IsMaximized) === true

                text: i18n("Ma&ximize")
                icon: "window-maximize"

                onClicked: tasksModel.requestToggleMaximized(modelIndex)
            }

            PlasmaComponents.MenuItem {
                visible: (menu.visualParent && get(atm.IsLauncher) !== true && get(atm.IsStartup) !== true)

                enabled: menu.visualParent && get(atm.IsMinimizable) === true

                checkable: true
                checked: menu.visualParent && get(atm.IsMinimized) === true

                text: i18n("Mi&nimize")
                icon: "window-minimize"

                onClicked: tasksModel.requestToggleMinimized(modelIndex)
            }

            PlasmaComponents.MenuItem {
                checkable: true
                checked: menu.visualParent && menu.get(atm.IsKeepAbove) === true

                text: i18n("Keep &Above Others")
                icon: "window-keep-above"

                onClicked: tasksModel.requestToggleKeepAbove(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                checkable: true
                checked: menu.visualParent && menu.get(atm.IsKeepBelow) === true

                text: i18n("Keep &Below Others")
                icon: "window-keep-below"

                onClicked: tasksModel.requestToggleKeepBelow(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                enabled: menu.visualParent && menu.get(atm.IsFullScreenable) === true

                checkable: true
                checked: menu.visualParent && menu.get(atm.IsFullScreen) === true

                text: i18n("&Fullscreen")
                icon: "view-fullscreen"

                onClicked: tasksModel.requestToggleFullScreen(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                enabled: menu.visualParent && menu.get(atm.IsShadeable) === true

                checkable: true
                checked: menu.visualParent && menu.get(atm.IsShaded) === true

                text: i18n("&Shade")
                icon: "window-shade"

                onClicked: tasksModel.requestToggleShaded(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                separator: true
            }

            PlasmaComponents.MenuItem {
                visible: (plasmoid.configuration.groupingStrategy !== 0) && menu.get(atm.IsWindow) === true

                checkable: true
                checked: menu.visualParent && menu.get(atm.IsGroupable) === true

                text: i18n("Allow this program to be grouped")
                icon: "view-group"

                onClicked: tasksModel.requestToggleGrouping(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                separator: true
            }

            PlasmaComponents.MenuItem {
                property QtObject configureAction: null

                enabled: configureAction && configureAction.enabled
                visible: configureAction && configureAction.visible

                text: configureAction ? configureAction.text : ""
                icon: configureAction ? configureAction.icon : ""

                onClicked: configureAction.trigger()

                Component.onCompleted: configureAction = plasmoid.action("configure")
            }

            PlasmaComponents.MenuItem {
                property QtObject alternativesAction: null

                enabled: alternativesAction && alternativesAction.enabled
                visible: alternativesAction && alternativesAction.visible

                text: alternativesAction ? alternativesAction.text : ""
                icon: alternativesAction ? alternativesAction.icon : ""

                onClicked: alternativesAction.trigger()

                Component.onCompleted: alternativesAction = plasmoid.action("alternatives")
            }
        }
    }

    PlasmaComponents.MenuItem {
        id: launcherToggleAction

        visible: visualParent
                     && get(atm.IsLauncher) !== true
                     && get(atm.IsStartup) !== true
                     && plasmoid.immutability !== PlasmaCore.Types.SystemImmutable
                     && (activityInfo.numberOfRunningActivities < 2)
                     && tasksModel.launcherPosition(get(atm.LauncherUrlWithoutIcon)) == -1

        enabled: visualParent && get(atm.LauncherUrlWithoutIcon) != ""

        text: i18n("&Pin to Task Manager")
        icon: "window-pin"

        onClicked: {
            if (tasksModel.launcherPosition(get(atm.LauncherUrlWithoutIcon)) !== -1) {
                tasksModel.requestRemoveLauncher(get(atm.LauncherUrlWithoutIcon));
            } else {
                tasksModel.requestAddLauncher(get(atm.LauncherUrl));
            }
        }
    }

    PlasmaComponents.MenuItem {
        id: showLauncherInActivitiesItem

        text: i18n("&Pin to Task Manager")
        icon: "window-pin"

        visible: visualParent
                     && get(atm.IsLauncher) !== true
                     && get(atm.IsStartup) !== true
                     && plasmoid.immutability !== PlasmaCore.Types.SystemImmutable
                     && (activityInfo.numberOfRunningActivities >= 2)

        Connections {
            target: activityInfo
            function onNumberOfRunningActivitiesChanged() {
                activitiesDesktopsMenu.refresh()
            }
        }

        PlasmaComponents.ContextMenu {
            id: activitiesLaunchersMenu
            visualParent: showLauncherInActivitiesItem.action

            function refresh() {
                clearMenuItems();

                if (menu.visualParent === null) return;

                var createNewItem = function(id, title, url, activities) {
                    var result = menu.newMenuItem(activitiesLaunchersMenu);
                    result.text = title;

                    result.visible = true;
                    result.checkable = true;

                    result.checked = activities.some(function(activity) { return activity === id });

                    result.clicked.connect(
                        function() {
                            if (result.checked) {
                                tasksModel.requestAddLauncherToActivity(url, id);
                            } else {
                                tasksModel.requestRemoveLauncherFromActivity(url, id);
                            }
                        }
                    );

                    return result;
                }

                if (menu.visualParent === null) return;

                var url = menu.get(atm.LauncherUrlWithoutIcon);

                var activities = tasksModel.launcherActivities(url);

                var NULL_UUID = "00000000-0000-0000-0000-000000000000";

                createNewItem(NULL_UUID, i18n("On All Activities"), url, activities);

                if (activityInfo.numberOfRunningActivities <= 1) {
                    return;
                }

                createNewItem(activityInfo.currentActivity, i18n("On The Current Activity"), url, activities);

                menu.newSeparator(activitiesLaunchersMenu);

                var runningActivities = activityInfo.runningActivities();

                runningActivities.forEach(function(id) {
                    createNewItem(id, activityInfo.activityName(id), url, activities);
                });
            }

            Component.onCompleted: {
                menu.onVisualParentChanged.connect(refresh);
                refresh();
            }
        }
    }

    PlasmaComponents.MenuItem {
        visible: (visualParent
                && plasmoid.immutability !== PlasmaCore.Types.SystemImmutable
                && !launcherToggleAction.visible
                && !showLauncherInActivitiesItem.visible)

        text: i18n("Unpin from Task Manager")
        icon: "window-unpin"

        onClicked: {
            tasksModel.requestRemoveLauncher(get(atm.LauncherUrlWithoutIcon));
        }
    }

    PlasmaComponents.MenuItem { separator: true }

    PlasmaComponents.MenuItem {
        id: closeWindowItem
        visible: (visualParent && get(atm.IsLauncher) !== true && get(atm.IsStartup) !== true)

        enabled: visualParent && get(atm.IsClosable) === true

        text: i18n("&Close")
        icon: "window-close"

        onClicked: tasksModel.requestClose(modelIndex)
    }
}
