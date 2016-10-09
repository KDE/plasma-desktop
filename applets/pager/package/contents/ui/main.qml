/*
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
 * Copyright 2016  Kai Uwe Broulik <kde@privat.broulik.de>
 * Copyright 2016  Eike Hein <hein@kde.org>
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

    property bool isActivityPager: (plasmoid.pluginName == "org.kde.plasma.activitypager")
    property bool vertical: (plasmoid.formFactor == PlasmaCore.Types.Vertical)
    property var activityDataSource: null

    Layout.minimumWidth: !root.vertical ? pagerItemGrid.width : 1
    Layout.minimumHeight: root.vertical ? pagerItemGrid.height : 1

    Layout.maximumWidth: !root.vertical ? pagerItemGrid.width : Infinity
    Layout.maximumHeight: root.vertical ? pagerItemGrid.height : Infinity

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.status: pagerModel.shouldShowPager ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus

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
        pagerModel.addDesktop();
    }

    function action_removeDesktop() {
        pagerModel.removeDesktop();
    }

    function action_openKCM() {
        KQuickControlsAddonsComponents.KCMShell.open("desktop");
    }

    function action_showActivityManager() {
        if (!activityDataSource) {
            activityDataSource = Qt.createQmlObject('import org.kde.plasma.core 2.0 as PlasmaCore; \
                PlasmaCore.DataSource { id: dataSource; engine: "org.kde.activities"; \
                connectedSources: ["Status"] }', root);
        }

        var service = activityDataSource.serviceForSource("Status")
        var operation = service.operationDescription("toggleActivityManager")
        service.startOperationCall(operation)
    }

    onWheel: {
        if (wheel.angleDelta.y > 0 || wheel.angleDelta.x > 0) {
            pagerModel.changePage((repeater.count + pagerModel.currentPage - 2) % repeater.count);
        } else {
            pagerModel.changePage(pagerModel.currentPage % repeater.count);
        }
    }

    PagerModel {
        id: pagerModel

        enabled: root.visible

        showDesktop: (plasmoid.configuration.currentDesktopSelected == 1)

        showOnlyCurrentScreen: plasmoid.configuration.showOnlyCurrentScreen
        screenGeometry: plasmoid.screenGeometry

        pagerType: isActivityPager ? PagerModel.Activities : PagerModel.VirtualDesktops
    }

    Connections {
        target: plasmoid.configuration

        onShowWindowIconsChanged: {
            // Causes the model to reset; Component.onCompleted in the
            // window delegate now gets a chance to create the icon item,
            // which it otherwise will not do.
            pagerModel.refresh();
        }

        onDisplayedTextChanged: {
            // Causes the model to reset; Component.onCompleted in the
            // desktop delegate now gets a chance to create the label item,
            // which it otherwise will not do.
            pagerModel.refresh();
        }
    }

    Component {
        id: desktopLabelComponent

        PlasmaComponents.Label {
            anchors {
                fill: parent
                topMargin: desktopFrame.margins.top
                bottomMargin: desktopFrame.margins.bottom
                leftMargin: desktopFrame.margins.left
                rightMargin: desktopFrame.margins.right
            }

            property int index: 0
            property var model: null
            property Item desktopFrame: null

            text: plasmoid.configuration.displayedText ? model.display : index + 1

            wrapMode: Text.NoWrap
            elide: Text.ElideRight

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    Component {
        id: windowIconComponent

        PlasmaCore.IconItem {
            anchors.centerIn: parent

            height: parent.width / 2
            width: parent.height / 2

            property var model: null

            source: model ? model.decoration : undefined
            usesPlasmaTheme: false
            animated: false
        }
    }

    Timer {
        id: dragTimer
        interval: 1000
        onTriggered: {
            if (dragSwitchDesktopId != -1 && dragSwitchDesktopId !== pagerModel.currentPage - 1) {
                pagerModel.changePage(dragSwitchDesktopId);
            }
        }
    }

    Grid {
        id: pagerItemGrid

        spacing: units.devicePixelRatio
        rows: effectiveRows
        columns: effectiveColumns

        readonly property int effectiveRows: {
            if (!pagerModel.count) {
                return 1;
            }

            var columns = Math.floor(pagerModel.count / pagerModel.layoutRows);

            if (pagerModel.count % pagerModel.layoutRows > 0) {
                columns += 1;
            }

            var rows = Math.floor(pagerModel.count / columns);

            if (pagerModel.count % columns > 0) {
                rows += 1;
            }

            return rows;
        }

        readonly property int effectiveColumns: {
            if (!pagerModel.count) {
                return 1;
            }

            return Math.ceil(pagerModel.count / effectiveRows);
        }

        readonly property real pagerItemSizeRatio: pagerModel.pagerItemSize.width / pagerModel.pagerItemSize.height
        readonly property real widthScaleFactor: columnWidth / pagerModel.pagerItemSize.width
        readonly property real heightScaleFactor: rowHeight / pagerModel.pagerItemSize.height


        states: [
            State {
                name: "vertical"
                when: vertical
                PropertyChanges {
                    target: pagerItemGrid
                    innerSpacing: effectiveColumns
                    rowHeight: Math.floor(columnWidth / pagerItemSizeRatio)
                    columnWidth: Math.floor((root.width - innerSpacing) / effectiveColumns)
                }
            }
        ]

        property int innerSpacing: (effectiveRows - 1) * spacing
        property int rowHeight: Math.floor((root.height - innerSpacing) / effectiveRows)
        property int columnWidth: Math.floor(rowHeight * pagerItemSizeRatio)

        Repeater {
            id: repeater
            model: pagerModel

            PlasmaCore.ToolTipArea {
                id: desktop

                property int desktopId: index
                property bool active: isActivityPager ? (index == pagerModel.currentPage) : (index + 1 == pagerModel.currentPage)

                mainText: model.display
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

                width: pagerItemGrid.columnWidth
                height: pagerItemGrid.rowHeight

                PlasmaCore.FrameSvgItem {
                    id: desktopFrame

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
                        pagerModel.drop(event.mimeData, desktop.desktopId);
                        root.dragSwitchDesktopId = -1;
                        dragTimer.stop();
                    }
                }

                MouseArea {
                    id: desktopMouseArea
                    anchors.fill: parent
                    hoverEnabled : true
                    onClicked: pagerModel.changePage(desktopId);
                }

                Item {
                    id: clipRect
                    x: Math.round(units.devicePixelRatio)
                    y: Math.round(units.devicePixelRatio)
                    width: desktop.width - 2 * x
                    height: desktop.height - 2 * y
                    clip: true


                    Repeater {
                        id: windowRectRepeater

                        model: TasksModel

                        onCountChanged: desktop.updateSubText()

                        Rectangle {
                            id: windowRect

                            z: model.StackingOrder

                            property rect geometry: model.Geometry
                            property int windowId: model.LegacyWinIdList[0]
                            property string visibleName: model.display
                            property bool minimized: (model.IsMinimized === true)
                            onMinimizedChanged: desktop.updateSubText()

                            /* since we move clipRect with 1, move it back */
                            x: (geometry.x * pagerItemGrid.widthScaleFactor) - Math.round(units.devicePixelRatio)
                            y: (geometry.y * pagerItemGrid.heightScaleFactor) - Math.round(units.devicePixelRatio)
                            width: geometry.width * pagerItemGrid.widthScaleFactor
                            height: geometry.height * pagerItemGrid.heightScaleFactor
                            visible: model.IsMinimized !== true
                            color: {
                                if (desktop.active) {
                                    if (model.IsActive === true)
                                        return windowActiveOnActiveDesktopColor;
                                    else
                                        return windowInactiveOnActiveDesktopColor;
                                } else {
                                    if (model.IsActive === true)
                                        return windowActiveColor;
                                    else
                                        return windowInactiveColor;
                                }
                            }

                            border.width: Math.round(units.devicePixelRatio)
                            border.color: (model.IsActive === true) ? windowActiveBorderColor
                                                    : windowInactiveBorderColor

                            MouseArea {
                                id: windowMouseArea
                                anchors.fill: parent

                                drag.target: windowRect
                                drag.axis: Drag.XandYAxis
                                drag.minimumX: -windowRect.width/2
                                drag.maximumX: root.width - windowRect.width/2
                                drag.minimumY: -windowRect.height/2
                                drag.maximumY: root.height - windowRect.height/2

                                drag.onActiveChanged: {
                                    root.dragging = drag.active;
                                    root.dragId = desktop.desktopId;
                                    desktopMouseArea.enabled = !drag.active;

                                    if (drag.active) {
                                        // Reparent to allow drags outside of this desktop.
                                        var value = root.mapFromItem(clipRect, windowRect.x, windowRect.y);
                                        windowRect.parent = root;
                                        windowRect.x = value.x;
                                        windowRect.y = value.y
                                    }
                                }

                                onReleased: {
                                    if (root.dragging) {
                                        windowRect.visible = false;
                                        var windowCenter = Qt.point(windowRect.x + windowRect.width / 2, windowRect.y + windowRect.height / 2);
                                        var pagerItem = pagerItemGrid.childAt(windowCenter.x, windowCenter.y);

                                        if (pagerItem) {
                                            var relativeTopLeft = root.mapToItem(pagerItem, windowRect.x, windowRect.y);

                                            pagerModel.moveWindow(windowRect.windowId, relativeTopLeft.x, relativeTopLeft.y,
                                                pagerItem.desktopId, root.dragId,
                                                pagerItemGrid.widthScaleFactor, pagerItemGrid.heightScaleFactor);
                                        }

                                        // Will reset the model, destroying the reparented drag delegate that
                                        // is no longer bound to model.Geometry.
                                        root.dragging = false;
                                        pagerModel.refresh();
                                    } else {
                                        // When there is no dragging (just a click), the event is passed
                                        // to the desktop MouseArea.
                                        desktopMouseArea.clicked(mouse);
                                    }
                                }
                            }

                            Component.onCompleted: {
                                if (plasmoid.configuration.showWindowIcons) {
                                    windowIconComponent.createObject(windowRect, {"model": model});
                                }
                            }
                        }
                    }
                }

                Component.onCompleted: {
                    if (plasmoid.configuration.displayedText < 2) {
                        desktopLabelComponent.createObject(desktop, {"index": index, "model": model, "desktopFrame": desktopFrame});
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        if (isActivityPager) {
            plasmoid.setAction("showActivityManager", i18n("Show Activity Manager..."), "preferences-activities");
        } else {
            if (KQuickControlsAddonsComponents.KCMShell.authorize("desktop.desktop").length > 0) {
                plasmoid.setAction("addDesktop", i18n("Add Virtual Desktop"), "list-add");
                plasmoid.setAction("removeDesktop", i18n("Remove Virtual Desktop"), "list-remove");
                plasmoid.action("removeDesktop").enabled = Qt.binding(function() {
                    return repeater.count > 1;
                });

                plasmoid.setAction("openKCM", i18n("Configure Desktops..."), "configure");
            }
        }
    }
}
