/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.kquickcontrolsaddons 2.0

PlasmaCore.ToolTipArea {
    id: root
    objectName: "org.kde.desktop-CompactApplet"
    anchors.fill: parent

    mainText: Plasmoid.toolTipMainText
    subText: Plasmoid.toolTipSubText
    location: Plasmoid.location
    active: !Plasmoid.expanded
    textFormat: Plasmoid.toolTipTextFormat
    mainItem: Plasmoid.toolTipItem ? Plasmoid.toolTipItem : null

    property Item fullRepresentation
    property Item compactRepresentation
    property Item expandedFeedback: expandedItem

    onCompactRepresentationChanged: {
        if (compactRepresentation) {
            compactRepresentation.anchors.fill = null;
            compactRepresentation.parent = compactRepresentationParent;
            compactRepresentation.anchors.fill = compactRepresentationParent;
            compactRepresentation.visible = true;
        }
        root.visible = true;
    }

    onFullRepresentationChanged: {
        if (fullRepresentation) {
            fullRepresentation.anchors.fill = null;
            fullRepresentation.parent = appletParent;
            fullRepresentation.anchors.fill = appletParent;
        }
    }

    FocusScope {
        id: compactRepresentationParent
        anchors.fill: parent
        activeFocusOnTab: true
        onActiveFocusChanged: {
            // When the scope gets the active focus, try to focus its first descendant,
            // if there is on which has activeFocusOnTab
            if (!activeFocus) {
                return;
            }
            let nextItem = nextItemInFocusChain();
            let candidate = nextItem;
            while (candidate.parent) {
                if (candidate === compactRepresentationParent) {
                    nextItem.forceActiveFocus();
                    return;
                }
                candidate = candidate.parent;
            }
        }

        objectName: "expandApplet"
        Accessible.name: root.mainText
        Accessible.description: i18nd("plasma_shell_org.kde.plasma.desktop", "Open %1", root.subText)
        Accessible.role: Accessible.Button
        Accessible.onPressAction: Plasmoid.nativeInterface.activated()

        Keys.onPressed: {
            switch (event.key) {
                case Qt.Key_Space:
                case Qt.Key_Enter:
                case Qt.Key_Return:
                case Qt.Key_Select:
                    Plasmoid.nativeInterface.activated();
                    break;
            }
        }
    }

    PlasmaCore.FrameSvgItem {
        id: expandedItem
        z: -100

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
            let prefix;
            switch (Plasmoid.location) {
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
        opacity: Plasmoid.expanded ? 1 : 0
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
        onTriggered: Plasmoid.expanded = dialog.visible;
    }

    Connections {
        target: Plasmoid.action("configure")
        function onTriggered() {
            if (Plasmoid.hideOnWindowDeactivate) {
                Plasmoid.expanded = false
            }
        }
    }

    Connections {
        target: Plasmoid.self
        function onContextualActionsAboutToShow() { root.hideImmediately() }
    }

    PlasmaCore.Dialog {
        id: dialog
        objectName: "popupWindow"
        flags: Qt.WindowStaysOnTopHint
        location: Plasmoid.location
        hideOnWindowDeactivate: Plasmoid.hideOnWindowDeactivate
        visible: Plasmoid.expanded && fullRepresentation
        visualParent: root.compactRepresentation
        backgroundHints: (Plasmoid.containmentDisplayHints & PlasmaCore.Types.DesktopFullyCovered) ? PlasmaCore.Dialog.SolidBackground : PlasmaCore.Dialog.StandardBackground
        type: PlasmaCore.Dialog.AppletPopup
        appletInterface: fullRepresentation && fullRepresentation.appletInterface || null

        property var oldStatus: PlasmaCore.Types.UnknownStatus

        onVisibleChanged: {
            if (!visible) {
                expandedSync.restart();
                Plasmoid.status = oldStatus;
            } else {
                oldStatus = Plasmoid.status;
                Plasmoid.status = PlasmaCore.Types.RequiresAttentionStatus;
                // This call currently fails and complains at runtime:
                // QWindow::setWindowState: QWindow::setWindowState does not accept Qt::WindowActive
                dialog.requestActivate();
            }
        }
        //It's a MouseEventListener to get all the events, so the eventfilter will be able to catch them
        mainItem: MouseEventListener {
            id: appletParent

            focus: true

            Keys.onEscapePressed: {
                Plasmoid.expanded = false;
            }

            Layout.minimumWidth: fullRepresentation ? fullRepresentation.Layout.minimumWidth : 0
            Layout.minimumHeight: fullRepresentation ? fullRepresentation.Layout.minimumHeight : 0

            Layout.preferredWidth: fullRepresentation ? fullRepresentation.Layout.preferredWidth : -1
            Layout.preferredHeight: fullRepresentation ? fullRepresentation.Layout.preferredHeight : -1

            Layout.maximumWidth: fullRepresentation ? fullRepresentation.Layout.maximumWidth : Infinity
            Layout.maximumHeight: fullRepresentation ? fullRepresentation.Layout.maximumHeight : Infinity

            width: {
                if (root.fullRepresentation !== null) {
                    /****/ if (root.fullRepresentation.Layout.preferredWidth > 0) {
                        return root.fullRepresentation.Layout.preferredWidth;
                    } else if (root.fullRepresentation.implicitWidth > 0) {
                        return root.fullRepresentation.implicitWidth;
                    }
                }
                return PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 35;
            }
            height: {
                if (root.fullRepresentation !== null) {
                    /****/ if (fullRepresentation.Layout.preferredHeight > 0) {
                        return fullRepresentation.Layout.preferredHeight;
                    } else if (fullRepresentation.implicitHeight > 0) {
                        return fullRepresentation.implicitHeight;
                    }
                }
                return PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height * 25;
            }

            onActiveFocusChanged: {
                if (activeFocus && fullRepresentation) {
                    fullRepresentation.forceActiveFocus()
                }
            }

            // Draws a line between the applet dialog and the panel
            PlasmaCore.SvgItem {
                // Only draw for popups of panel applets, not desktop applets
                visible: [PlasmaCore.Types.TopEdge, PlasmaCore.Types.LeftEdge, PlasmaCore.Types.RightEdge, PlasmaCore.Types.BottomEdge]
                    .includes(Plasmoid.location)
                anchors {
                    top: Plasmoid.location === PlasmaCore.Types.BottomEdge ? undefined : parent.top
                    left: Plasmoid.location === PlasmaCore.Types.RightEdge ? undefined : parent.left
                    right: Plasmoid.location === PlasmaCore.Types.LeftEdge ? undefined : parent.right
                    bottom: Plasmoid.location === PlasmaCore.Types.TopEdge ? undefined : parent.bottom
                    topMargin: Plasmoid.location === PlasmaCore.Types.BottomEdge ? undefined : -dialog.margins.top
                    leftMargin: Plasmoid.location === PlasmaCore.Types.RightEdge ? undefined : -dialog.margins.left
                    rightMargin: Plasmoid.location === PlasmaCore.Types.LeftEdge ? undefined : -dialog.margins.right
                    bottomMargin: Plasmoid.location === PlasmaCore.Types.TopEdge ? undefined : -dialog.margins.bottom
                }
                height: (Plasmoid.location === PlasmaCore.Types.TopEdge || Plasmoid.location === PlasmaCore.Types.BottomEdge) ? PlasmaCore.Units.devicePixelRatio : undefined
                width: (Plasmoid.location === PlasmaCore.Types.LeftEdge || Plasmoid.location === PlasmaCore.Types.RightEdge) ? PlasmaCore.Units.devicePixelRatio : undefined
                z: 999 /* Draw the line on top of the applet */
                elementId: (Plasmoid.location === PlasmaCore.Types.TopEdge || Plasmoid.location === PlasmaCore.Types.BottomEdge) ? "horizontal-line" : "vertical-line"
                svg: PlasmaCore.Svg {
                    imagePath: "widgets/line"
                }
            }

            LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
            LayoutMirroring.childrenInherit: true
        }
    }
}
