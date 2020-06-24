/**
 * SPDX-FileCopyrightText: (C) 2014 Weng Xuetian <wengxt@gmail.com>
 * SPDX-FileCopyrightText: (C) 2013-2017 Eike Hein <hein@kde.org>                   *
 * SPDX-FileCopyrightText: (C) 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.4
import QtQuick.Layouts 1.14
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kcoreaddons 1.0 as KCoreAddons
import org.kde.kirigami 2.0 as Kirigami
import QtGraphicalEffects 1.0

import org.kde.plasma.private.kicker 0.1 as Kicker

PlasmaCore.Dialog {
    id: root

    objectName: "popupWindow"
    flags: Qt.WindowStaysOnTopHint
    location: PlasmaCore.Types.Floating
    hideOnWindowDeactivate: true

    property int iconSize: units.iconSizes.huge
    property int cellSize: iconSize + theme.mSize(theme.defaultFont).height
        + (2 * units.smallSpacing)
        + (2 * Math.max(highlightItemSvg.margins.top + highlightItemSvg.margins.bottom,
                        highlightItemSvg.margins.left + highlightItemSvg.margins.right))
    property bool searching: searchField.text.length > 0

    onVisibleChanged: {
        if (!visible) {
            reset();
        } else {
            var pos = popupPosition(width, height);
            x = pos.x;
            y = pos.y;
            requestActivate();
        }
    }

    onHeightChanged: {
        var pos = popupPosition(width, height);
        x = pos.x;
        y = pos.y;
    }

    onWidthChanged: {
        var pos = popupPosition(width, height);
        x = pos.x;
        y = pos.y;
    }

    function popupPosition(width, height) {
        var screenAvail = plasmoid.availableScreenRect;
        var screenGeom = plasmoid.screenGeometry;
        //QtBug - QTBUG-64115
        var screen = Qt.rect(screenAvail.x + screenGeom.x,
                             screenAvail.y + screenGeom.y,
                             screenAvail.width,
                             screenAvail.height);

        var offset = units.gridUnit;

        // Fall back to bottom-left of screen area when the applet is on the desktop or floating.
        var x = offset;
        var y = screen.height - height - offset;

        if (plasmoid.location == PlasmaCore.Types.BottomEdge) {
            var horizMidPoint = screen.x + (screen.width / 2);
            var appletTopLeft = parent.mapToGlobal(0, 0);
            x = (appletTopLeft.x < horizMidPoint) ? screen.x + offset : (screen.x + screen.width) - width - offset;
            y = screen.height - height - offset - panelSvg.margins.top;
        } else if (plasmoid.location == PlasmaCore.Types.TopEdge) {
            var horizMidPoint = screen.x + (screen.width / 2);
            var appletBottomLeft = parent.mapToGlobal(0, parent.height);
            x = (appletBottomLeft.x < horizMidPoint) ? screen.x + offset : (screen.x + screen.width) - width - offset;
            y = parent.height + panelSvg.margins.bottom + offset;
        } else if (plasmoid.location == PlasmaCore.Types.LeftEdge) {
            var vertMidPoint = screen.y + (screen.height / 2);
            var appletTopLeft = parent.mapToGlobal(0, 0);
            x = parent.width + panelSvg.margins.right + offset;
            y = (appletTopLeft.y < vertMidPoint) ? screen.y + offset : (screen.y + screen.height) - height - offset;
        } else if (plasmoid.location == PlasmaCore.Types.RightEdge) {
            var vertMidPoint = screen.y + (screen.height / 2);
            var appletTopLeft = parent.mapToGlobal(0, 0);
            x = appletTopLeft.x - panelSvg.margins.left - offset - width;
            y = (appletTopLeft.y < vertMidPoint) ? screen.y + offset : (screen.y + screen.height) - height - offset;
        }

        return Qt.point(x, y);
    }
    
    
    FocusScope {
        // HACK headerUserInfo width + gridView width
        // Trying to make the size depends on the value of the two elements cause a crash when
        // hiding the dialog.
        Layout.minimumWidth: Kirigami.Units.gridUnit * 12 + cellSize * 6 + Kirigami.Units.gridUnit
        Layout.maximumWidth: Kirigami.Units.gridUnit * 12 + cellSize * 6 + Kirigami.Units.gridUnit
        
        Layout.minimumHeight: 500
        Layout.maximumHeight: 500
        Layout.alignment: Qt.AlignTop
        
        GridLayout {
            anchors.fill: parent
            
            columns: 3
            
            PlasmaExtras.PlasmoidHeading {
                id: headingUserInfo
                Layout.minimumWidth: Kirigami.Units.gridUnit * 12
                Layout.maximumWidth: Kirigami.Units.gridUnit * 12
                Layout.maximumHeight: Kirigami.Units.gridUnit * 2.5
                Layout.minimumHeight: Kirigami.Units.gridUnit * 2.5
                Layout.fillWidth: false
                KCoreAddons.KUser {
                    id: kuser
                }
                
                RowLayout {
                    id: profileInformation
                    Image {
                        id: faceIcon
                        source: kuser.faceIconUrl
                        cache: false
                        visible: source !== ""

                        width: Kirigami.Units.gridUnit * 2
                        height: width

                        sourceSize.width: width
                        sourceSize.height: height

                        fillMode: Image.PreserveAspectFit
                    }

                    PlasmaExtras.Heading {
                        text: kuser.fullName
                    }
                }
            }
            
            PlasmaCore.SvgItem {
                elementId: "vertical-line"
                svg: PlasmaCore.Svg {
                    imagePath: "widgets/line"
                }
                
                Layout.maximumHeight: Kirigami.Units.gridUnit * 2.5
                Layout.minimumHeight: Kirigami.Units.gridUnit * 2.5
                Layout.maximumWidth: 2
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
        
            PlasmaExtras.PlasmoidHeading {
                Layout.maximumWidth: pageListScrollArea.width
                Layout.maximumHeight: Kirigami.Units.gridUnit * 2.5
                Layout.minimumHeight: Kirigami.Units.gridUnit * 2.5
                SearchField {
                    id: searchField
                }
            }
            
            PlasmaExtras.ScrollArea {
                id: filterListScrollArea
                
                property int desiredWidth: 0
                property alias currentIndex: filterList.currentIndex

                width: plasmoid.configuration.showFilterList ? desiredWidth : 0

                enabled: !searching
                visible: plasmoid.configuration.showFilterList

                opacity: root.visible ? (searching ? 0.30 : 1.0) : 0.3

                Behavior on opacity { SmoothedAnimation { duration: units.longDuration; velocity: 0.01 } }

                verticalScrollBarPolicy: (opacity == 1.0) ? Qt.ScrollBarAsNeeded : Qt.ScrollBarAlwaysOff
                
                Layout.fillHeight: true
                Layout.minimumWidth: Kirigami.Units.gridUnit * 12
                Layout.maximumWidth: Kirigami.Units.gridUnit * 12

                onEnabledChanged: {
                    if (!enabled) {
                        filterList.currentIndex = -1;
                    }
                }

                ListView {
                    id: filterList

                    focus: true

                    property bool allApps: false
                    property int eligibleWidth: width
                    property int hItemMargins: highlightItemSvg.margins.left + highlightItemSvg.margins.right
                    model: filterListScrollArea.visible ? rootModel : null

                    boundsBehavior: Flickable.StopAtBounds
                    snapMode: ListView.SnapToItem
                    spacing: 0
                    keyNavigationWraps: true

                    delegate: ListItemDelegate {}

                    highlight: PlasmaComponents.Highlight {
                        anchors {
                            top: filterList.currentItem ? filterList.currentItem.top : undefined
                            left: filterList.currentItem ? filterList.currentItem.left : undefined
                            bottom: filterList.currentItem ? filterList.currentItem.bottom : undefined
                        }

                        opacity: filterListScrollArea.focus ? 1.0 : 0.7

                        width: parent.width

                        visible: filterList.currentItem
                    }

                    highlightFollowsCurrentItem: false
                    highlightMoveDuration: 0
                    highlightResizeDuration: 0

                    onCountChanged: {
                        var width = 0;

                        for (var i = 0; i < rootModel.count; ++i) {
                            headingMetrics.text = rootModel.labelForRow(i);

                            if (headingMetrics.width > width) {
                                width = headingMetrics.width;
                            }
                        }

                        filterListScrollArea.desiredWidth = width + hItemMargins + units.gridUnit;
                    }

                    Keys.onPressed: {
                        if (event.key == Qt.Key_left) {
                            event.accepted = true;

                            var currentRow = Math.max(0, Math.ceil(currentItem.y / cellSize) - 1);

                            if (pageList.currentItem) {
                                pageList.currentItem.itemGrid.tryActivate(currentRow, 5);
                            }
                        } else if (event.key == Qt.Key_Tab) {
                            event.accepted = true;
                            searchField.focus = true;
                        } else if (event.key == Qt.Key_Backtab) {
                            event.accepted = true;
                            systemFavoritesGrid.tryActivate(0, 0);
                        }
                    }
                }
            }
            
            PlasmaCore.SvgItem {
                elementId: "vertical-line"
                svg: PlasmaCore.Svg {
                    imagePath: "widgets/line"
                }
                
                Layout.fillHeight: true
                Layout.maximumWidth: 2
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            
            PlasmaExtras.ScrollArea {
                id: pageListScrollArea

                Layout.maximumWidth: cellSize * 6 + Kirigami.Units.gridUnit
                Layout.minimumWidth: cellSize * 6 + Kirigami.Units.gridUnit
                Layout.rightMargin: units.largeSpacing * 2
                Layout.fillHeight: true

                focus: true

                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                verticalScrollBarPolicy: Qt.ScrollBarAsNeeded
                
                GridView {
                    id: gridView

                    anchors.fill: parent

                    cellWidth: cellSize
                    cellHeight: cellSize

                    //dragEnabled: index === 0

                    model: searching ? runnerModel.modelForRow(0) : rootModel.modelForRow(filterListScrollArea.visible ? filterList.currentIndex : 0)
                    
                    ActionMenu {
                        id: actionMenu

                        onActionClicked: {
                            visualParent.actionTriggered(actionId, actionArgument);
                        }
                    }
                    
                    delegate: ItemGridDelegate {
                        usesPlasmaTheme: usesPlasmaTheme
                        onClicked: {
                            if (mouse.button == Qt.RightButton)
 {
                                openActionMenu(mouseX, mouseY);
                            }
                        }
                    }
                        
                    onCurrentIndexChanged: {
                        if (currentIndex != -1 && !searching) {
                            pageListScrollArea.focus = true;
                            focus = true;
                        }
                    }

                    onCountChanged: {
                        if (searching && index == 0) {
                            currentIndex = 0;
                        }
                    }
                }
            }
            
            PlasmaExtras.PlasmoidHeading {
                id: tab
                location: PlasmaExtras.PlasmoidHeading.Location.Footer
                Layout.minimumHeight: Kirigami.Units.gridUnit * 2.5
                
                PlasmaComponents3.TabBar {
                    anchors.fill: parent
                    anchors.bottom: parent.bottom
                    PlasmaComponents3.TabButton {
                        text: i18n("Applications")
                    }
                    PlasmaComponents3.TabButton {
                        text: i18n("Files")
                    }
                }
            }
            
            PlasmaCore.SvgItem {
                elementId: "vertical-line"
                svg: PlasmaCore.Svg {
                    imagePath: "widgets/line"
                }
                
                Layout.maximumHeight: tab.height
                Layout.maximumWidth: 2
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            
            PlasmaExtras.PlasmoidHeading {
                Layout.minimumHeight: Kirigami.Units.gridUnit * 2.5
                
                location: PlasmaExtras.PlasmoidHeading.Location.Footer
                
                ListView {
                    orientation: Qt.Horizontal
                    model: systemFavorites
                
                    delegate: PlasmaComponents3.ToolButton {
                        icon.name: "shutdown"
                        text: display
                    }
                }
            }
        }
    }
    /*
    Component.onCompleted: {
        kicker.reset.connect(reset);
        dragHelper.dropped.connect(pageList.cycle);
    }*/
}
