/*
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
 * Copyright 2016  Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddonsComponents
import org.kde.draganddrop 2.0
import org.kde.plasma.private.pager 2.0
import "utils.js" as Utils

MouseArea {
    id: root

    property bool vertical: (plasmoid.formFactor == PlasmaCore.Types.Vertical)

    Layout.minimumWidth: !root.vertical ? pager.preferredSize.width : 1
    Layout.minimumHeight: root.vertical ? pager.preferredSize.height : 1

    Layout.maximumWidth: !root.vertical ? pager.preferredSize.width : Infinity
    Layout.maximumHeight: root.vertical ? pager.preferredSize.height : Infinity

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.status: pager.desktopCount > 1 ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus

    Layout.fillWidth: root.vertical 
    Layout.fillHeight: !root.vertical 

    property bool dragging: false
    property int dragId

    property int dragSwitchDesktopId: -1

    anchors.fill: parent
    acceptedButtons: Qt.NoButton

    function colorWithAlpha(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha)
    }

    readonly property color windowActiveOnActiveDesktopColor: colorWithAlpha(theme.textColor, 0.6)
    readonly property color windowInactiveOnActiveDesktopColor: colorWithAlpha(theme.textColor, 0.35)
    readonly property color windowActiveColor: colorWithAlpha(theme.textColor, 0.5)
    readonly property color windowActiveBorderColor: theme.textColor
    readonly property color windowInactiveColor: colorWithAlpha(theme.textColor, 0.17)
    readonly property color windowInactiveBorderColor: colorWithAlpha(theme.textColor, 0.5)

    function action_addDesktop() {
        pager.slotAddDesktop();
    }

    function action_removeDesktop() {
        pager.slotRemoveDesktop();
    }

    function action_openKCM() {
        KQuickControlsAddonsComponents.KCMShell.open("desktop")
    }

    onWheel: {
        if (wheel.angleDelta.y > 0 || wheel.angleDelta.x > 0) {
            pager.changeDesktop((repeater.count + pager.currentDesktop - 2) % repeater.count)
        } else {
            pager.changeDesktop(pager.currentDesktop % repeater.count)
        }
    }

    Component.onCompleted: {
        if (KQuickControlsAddonsComponents.KCMShell.authorize("desktop.desktop").length > 0) {
            plasmoid.setAction("addDesktop", i18n("Add Virtual Desktop"), "list-add");
            plasmoid.setAction("removeDesktop", i18n("Remove Virtual Desktop"), "list-remove");
            plasmoid.action("removeDesktop").enabled = Qt.binding(function() {
                return repeater.count > 1;
            });

            plasmoid.setAction("openKCM", i18n("Configure Desktops..."), "configure");
        }
    }

    Pager {
        id: pager
        // don't bother updating the models when we're not visible
        enabled: root.visible
        orientation: plasmoid.formFactor == PlasmaCore.Types.Vertical ? Qt.Vertical : Qt.Horizontal
        size: Qt.size(root.width, root.height)
        showWindowIcons: plasmoid.configuration.showWindowIcons
        currentDesktopSelected: plasmoid.configuration.currentDesktopSelected
        displayedText: plasmoid.configuration.displayedText
    }

    Timer {
        id: dragTimer
        interval: 1000
        onTriggered: {
            if (dragSwitchDesktopId != -1 && dragSwitchDesktopId !== pager.currentDesktop-1) {
                pager.changeDesktop(dragSwitchDesktopId);
            }
        }
    }

    Repeater {
        id: repeater
        model: pager.model

        PlasmaCore.ToolTipArea {
            id: desktop

            property int desktopId: index
            property string desktopName: model.desktopName ? model.desktopName : ""
            property bool active: (desktopId === pager.currentDesktop-1)

            mainText: desktopName
            // our ToolTip has maximumLineCount of 8 which doesn't fit but QML doesn't
            // respect that in RichText so we effectively can put in as much as we like :)
            // it also gives us more flexibility when it comes to styling the <li>
            textFormat: Text.RichText

            function updateSubText() {
                var generateWindowList = function windowList(windows) {
                    // if we have 5 windows, we would show "4 and another one" with the
                    // hint that there's 1 more taking the same amount of space than just showing it
                    var maximum = windows.length === 5 ? 5 : 4

                    var text = "<ul><li>"
                             + windows.slice(0, maximum).join("</li><li>")
                             + "</li></ul>"

                    if (windows.length > maximum) {
                        text += i18np("...and %1 other window", "...and %1 other windows", windows.length - maximum)
                    }

                    return text
                }

                var text = ""
                var visibleWindows = []
                var minimizedWindows = []

                for (var i = 0, length = windowRectRepeater.count; i < length; ++i) {
                    var window = windowRectRepeater.itemAt(i)
                    if (window) {
                        if (window.minimized) {
                            minimizedWindows.push(window.visibleName)
                        } else {
                            visibleWindows.push(window.visibleName)
                        }
                    }
                }

                if (visibleWindows.length) {
                    text += i18np("%1 Window:", "%1 Windows:", visibleWindows.length)
                          + generateWindowList(visibleWindows)
                }

                if (visibleWindows.length && minimizedWindows.length) {
                    text += "<br>"
                }

                if (minimizedWindows.length > 0) {
                    text += i18np("%1 Minimized Window:", "%1 Minimized Windows:", minimizedWindows.length)
                          + generateWindowList(minimizedWindows)
                }

                if (text.length) {
                    // Get rid of the spacing <ul> would cause
                    text = "<style>ul { margin: 0; }</style>" + text
                }

                subText = text
            }

            x: model.x
            y: model.y
            width: model.width
            height: model.height

            PlasmaCore.FrameSvgItem {
                anchors.fill: parent
                z: 1 // to make sure that the FrameSvg will be placed on top of the windows
                imagePath: "widgets/pager"
                prefix: (desktopMouseArea.enabled && desktopMouseArea.containsMouse) || (root.dragging && root.dragId == desktopId) ?
                            "hover" : (desktop.active ? "active" : "normal")
            }

            DropArea {
                id: droparea
                anchors.fill: parent
                preventStealing: true

                onDragEnter: {
                    root.dragSwitchDesktopId = desktop.desktopId;
                    dragTimer.start();
                }
                onDragLeave: {
                    root.dragSwitchDesktopId = -1;
                    dragTimer.stop();
                }
                onDrop: {
                    pager.dropMimeData(event.mimeData, desktop.desktopId);
                    root.dragSwitchDesktopId = -1;
                    dragTimer.stop();
                }
            }

            MouseArea {
                id: desktopMouseArea
                anchors.fill: parent
                hoverEnabled : true
                onClicked: pager.changeDesktop(desktopId);
            }

            Item {
                id: clipRect
                x: Math.round(units.devicePixelRatio)
                y: Math.round(units.devicePixelRatio)
                width: desktop.width - 2 * x
                height: desktop.height - 2 * y
                clip: true

                PlasmaComponents.Label {
                    id: desktopText
                    anchors.centerIn: parent
                    text: pager.displayedText == Pager.Name ? desktop.desktopName
                                                : (pager.displayedText == Pager.Number ? desktop.desktopId+1 : "")
                    
                }

                Repeater {
                    id: windowRectRepeater
                    model: windows
                    onCountChanged: desktop.updateSubText()

                    Rectangle {
                        id: windowRect

                        property int windowId: model.windowId
                        property string visibleName: model.visibleName
                        property bool minimized: model.minimized
                        onMinimizedChanged: desktop.updateSubText()

                        /* since we move clipRect with 1, move it back */
                        x: model.x - Math.round(units.devicePixelRatio)
                        y: model.y - Math.round(units.devicePixelRatio)
                        width: model.width
                        height: model.height
                        visible: !model.visible
                        color: {
                            if (desktop.active) {
                                if (model.active)
                                    return windowActiveOnActiveDesktopColor;
                                else
                                    return windowInactiveOnActiveDesktopColor;
                            } else {
                                if (model.active)
                                    return windowActiveColor;
                                else
                                    return windowInactiveColor;
                            }
                        }

                        border.width: Math.round(units.devicePixelRatio)
                        border.color: model.active ? windowActiveBorderColor
                                                   : windowInactiveBorderColor

                        KQuickControlsAddonsComponents.QPixmapItem {
                            id: icon
                            anchors.centerIn: parent
                            pixmap: model.icon
                            height: nativeHeight
                            width: nativeWidth
                            visible: pager.showWindowIcons
                        }

                        MouseArea {
                            id: windowMouseArea
                            anchors.fill: parent
                            drag.target: windowRect
                            drag.axis: Drag.XandYAxis
                            drag.minimumX: -windowRect.width/2
                            drag.maximumX: root.width - windowRect.width/2
                            drag.minimumY: -windowRect.height/2
                            drag.maximumY: root.height - windowRect.height/2

                            // used to save the state of some properties before the dragging
                            QtObject {
                                id: saveState
                                property int x: -1
                                property int y: -1
                                property variant parent
                                property int desktop: -1
                                property int mouseX: -1
                                property int mouseY: -1
                            }

                            drag.onActiveChanged: {
                                root.dragging = drag.active;
                                desktopMouseArea.enabled = !drag.active;
                            }

                            // reparent windowRect to enable the dragging for other desktops
                            onPressed: {
                                if (windowRect.parent == root)
                                    return;

                                saveState.x = windowRect.x;
                                saveState.y = windowRect.y
                                saveState.parent = windowRect.parent;
                                saveState.desktop = desktop.desktopId;
                                saveState.mouseX = mouseX;
                                saveState.mouseY = mouseY;

                                var value = root.mapFromItem(clipRect, windowRect.x, windowRect.y);
                                windowRect.x = value.x;
                                windowRect.y = value.y
                                windowRect.parent = root;
                            }

                            onReleased: {
                                if (root.dragging) {
                                    pager.moveWindow(windowRect.windowId, windowRect.x, windowRect.y,
                                                     root.dragId, saveState.desktop);
                                } else {
                                    // when there is no dragging (just a click), the event is passed
                                    // to the desktop mousearea
                                    desktopMouseArea.clicked(mouse);
                                }

                                windowRect.x = saveState.x;
                                windowRect.y = saveState.y;
                                windowRect.parent = saveState.parent;
                            }
                        }

                        function checkDesktopHover() {
                            if (!windowMouseArea.drag.active)
                                return;

                            var mouse = root.mapFromItem(windowRect, saveState.mouseX, saveState.mouseY);
                            for (var i = 0; i < root.children.length; i++) {
                                var item = root.children[i];
                                if (item.desktopId != undefined && Utils.contains(item, mouse)) {
                                    root.dragId = item.desktopId;
                                    return;
                                }
                            }
                        }

                        onXChanged: checkDesktopHover();
                        onYChanged: checkDesktopHover();
                    }
                }
            }
        }
    }
}
