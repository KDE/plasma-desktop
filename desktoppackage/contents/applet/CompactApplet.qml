/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

PlasmaCore.ToolTipArea {
    id: root
    objectName: "org.kde.desktop-CompactApplet"
    anchors.fill: parent

    mainText: plasmoid.toolTipMainText
    subText: plasmoid.toolTipSubText
    location: plasmoid.location
    active: !plasmoid.expanded
    textFormat: plasmoid.toolTipTextFormat
    mainItem: plasmoid.toolTipItem ? plasmoid.toolTipItem : null

    property Item fullRepresentation
    property Item compactRepresentation
    property Item expandedFeedback: expandedItem

    onCompactRepresentationChanged: {
        if (compactRepresentation) {
            compactRepresentation.parent = root;
            compactRepresentation.anchors.fill = root;
            compactRepresentation.visible = true;
        }
        root.visible = true;
    }

    onFullRepresentationChanged: {

        if (!fullRepresentation) {
            return;
        }

        //if the fullRepresentation size was restored to a stored size, or if is dragged from the desktop, restore popup size
        if (fullRepresentation.Layout && fullRepresentation.Layout.preferredWidth > 0) {
            popupWindow.mainItem.width = Qt.binding(function() {
                return fullRepresentation.Layout.preferredWidth
            })
        } else if (fullRepresentation.implicitWidth > 0) {
            popupWindow.mainItem.width = Qt.binding(function() {
                return fullRepresentation.implicitWidth
            })
        } else if (fullRepresentation.width > 0) {
            popupWindow.mainItem.width = Qt.binding(function() {
                return fullRepresentation.width
            })
        } else {
            popupWindow.mainItem.width = Qt.binding(function() {
                return PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 35
            })
        }

        if (fullRepresentation.Layout && fullRepresentation.Layout.preferredHeight > 0) {
            popupWindow.mainItem.height = Qt.binding(function() {
                return fullRepresentation.Layout.preferredHeight
            })
        } else if (fullRepresentation.implicitHeight > 0) {
            popupWindow.mainItem.height = Qt.binding(function() {
                return fullRepresentation.implicitHeight
            })
        } else if (fullRepresentation.height > 0) {
            popupWindow.mainItem.height = Qt.binding(function() {
                return fullRepresentation.height
            })
        } else {
            popupWindow.mainItem.height = Qt.binding(function() {
                return PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height * 25
            })
        }

        fullRepresentation.parent = appletParent;
        fullRepresentation.anchors.fill = fullRepresentation.parent;
    }

    PlasmaCore.FrameSvgItem {
        id: expandedItem

        property var containerMargins: {
            let item = root;
            while (item.parent) {
                item = item.parent;
                if (item.isAppletContainer) {
                    return item.getMargins;
                }
            }
            return undefined;
        }

        anchors {
            fill: parent
            property bool returnAllMargins: true 
            // The above makes sure margin is returned even for side margins, that 
            // would be otherwise turned off.
            bottomMargin: containerMargins ? -containerMargins('bottom', returnAllMargins) : 0;
            topMargin: containerMargins ? -containerMargins('top', returnAllMargins) : 0;
            leftMargin: containerMargins ? -containerMargins('left', returnAllMargins) : 0;
            rightMargin: containerMargins ? -containerMargins('right', returnAllMargins) : 0;
        }
        imagePath: "widgets/tabbar"
        visible: fromCurrentTheme && opacity > 0
        prefix: {
            var prefix;
            switch (plasmoid.location) {
                case PlasmaCore.Types.LeftEdge:
                    prefix = "west-active-tab";
                    break;
                case PlasmaCore.Types.TopEdge:
                    prefix = "north-active-tab";
                    break;
                case PlasmaCore.Types.RightEdge:
                    prefix = "east-active-tab";
                    break;
                default:
                    prefix = "south-active-tab";
                }
                if (!hasElementPrefix(prefix)) {
                    prefix = "active-tab";
                }
                return prefix;
            }
        opacity: plasmoid.expanded ? 1 : 0
        Behavior on opacity {
            NumberAnimation {
                duration: PlasmaCore.Units.shortDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    Timer {
        id: expandedSync
        interval: 100
        onTriggered: plasmoid.expanded = popupWindow.visible;
    }

    Connections {
        target: plasmoid.action("configure")
        function onTriggered() {
            if (plasmoid.hideOnWindowDeactivate) {
                plasmoid.expanded = false
            }
        }
    }

    Connections {
        target: plasmoid
        function onContextualActionsAboutToShow() { root.hideImmediately() }
    }

    PlasmaCore.Dialog {
        id: popupWindow
        objectName: "popupWindow"
        flags: Qt.WindowStaysOnTopHint
        visible: plasmoid.expanded && fullRepresentation
        visualParent: compactRepresentation ? compactRepresentation : null
        location: plasmoid.location
        hideOnWindowDeactivate: plasmoid.hideOnWindowDeactivate
        backgroundHints: (plasmoid.containmentDisplayHints & PlasmaCore.Types.DesktopFullyCovered) ? PlasmaCore.Dialog.SolidBackground : PlasmaCore.Dialog.StandardBackground

        property var oldStatus: PlasmaCore.Types.UnknownStatus

        //It's a MouseEventListener to get all the events, so the eventfilter will be able to catch them
        mainItem: MouseEventListener {
            id: appletParent

            focus: true

            Keys.onEscapePressed: {
                plasmoid.expanded = false;
            }

            LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
            LayoutMirroring.childrenInherit: true

            Layout.minimumWidth: (fullRepresentation && fullRepresentation.Layout) ? fullRepresentation.Layout.minimumWidth : 0
            Layout.minimumHeight: (fullRepresentation && fullRepresentation.Layout) ? fullRepresentation.Layout.minimumHeight: 0

            Layout.preferredWidth: (fullRepresentation && fullRepresentation.Layout) ? fullRepresentation.Layout.preferredWidth : -1
            Layout.preferredHeight: (fullRepresentation && fullRepresentation.Layout) ? fullRepresentation.Layout.preferredHeight: -1

            Layout.maximumWidth: (fullRepresentation && fullRepresentation.Layout) ? fullRepresentation.Layout.maximumWidth : Infinity
            Layout.maximumHeight: (fullRepresentation && fullRepresentation.Layout) ? fullRepresentation.Layout.maximumHeight: Infinity

            onActiveFocusChanged: {
                if (activeFocus && fullRepresentation) {
                    fullRepresentation.forceActiveFocus()
                }
            }

            // Draws a line between the applet dialog and the panel
            PlasmaCore.SvgItem {
                // Only draw for popups of panel applets, not desktop applets
                visible: plasmoid.formFactor === PlasmaCore.Types.Vertical || plasmoid.formFactor === PlasmaCore.Types.Horizontal

                anchors {
                    top: plasmoid.location == PlasmaCore.Types.BottomEdge ? undefined : parent.top
                    left: plasmoid.location == PlasmaCore.Types.RightEdge ? undefined : parent.left
                    right: plasmoid.location == PlasmaCore.Types.LeftEdge ? undefined : parent.right
                    bottom: plasmoid.location == PlasmaCore.Types.TopEdge ? undefined : parent.bottom
                    topMargin: plasmoid.location == PlasmaCore.Types.BottomEdge ? undefined : -popupWindow.margins.top
                    leftMargin: plasmoid.location == PlasmaCore.Types.RightEdge ? undefined : -popupWindow.margins.left
                    rightMargin: plasmoid.location == PlasmaCore.Types.LeftEdge ? undefined : -popupWindow.margins.right
                    bottomMargin: plasmoid.location == PlasmaCore.Types.TopEdge ? undefined : -popupWindow.margins.bottom
                }
                height: (plasmoid.location == PlasmaCore.Types.TopEdge || plasmoid.location == PlasmaCore.Types.BottomEdge) ? 1 : undefined
                width: (plasmoid.location == PlasmaCore.Types.LeftEdge || plasmoid.location == PlasmaCore.Types.RightEdge) ? 1 : undefined
                z: 999 /* Draw the line on top of the applet */
                elementId: (plasmoid.location == PlasmaCore.Types.TopEdge || plasmoid.location == PlasmaCore.Types.BottomEdge) ? "horizontal-line" : "vertical-line"
                svg: PlasmaCore.Svg {
                    imagePath: "widgets/line"
                }
            }
        }

        onVisibleChanged: {
            if (!visible) {
                expandedSync.restart();
                plasmoid.status = oldStatus;
            } else {
                oldStatus = plasmoid.status;
                plasmoid.status = PlasmaCore.Types.RequiresAttentionStatus;
                // This call currently fails and complains at runtime:
                // QWindow::setWindowState: QWindow::setWindowState does not accept Qt::WindowActive
                popupWindow.requestActivate();
            }
        }

    }
}
