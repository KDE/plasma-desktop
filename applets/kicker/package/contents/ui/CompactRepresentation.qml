/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.plasma.private.kicker 0.1 as Kicker

Item {
    id: root

    readonly property bool inPanel: (plasmoid.location === PlasmaCore.Types.TopEdge
        || plasmoid.location === PlasmaCore.Types.RightEdge
        || plasmoid.location === PlasmaCore.Types.BottomEdge
        || plasmoid.location === PlasmaCore.Types.LeftEdge)
    readonly property bool vertical: (plasmoid.formFactor === PlasmaCore.Types.Vertical)
    readonly property bool useCustomButtonImage: (plasmoid.configuration.useCustomButtonImage
        && plasmoid.configuration.customButtonImage.length !== 0)

    readonly property Component dashWindowComponent: kicker.isDash ? Qt.createComponent(Qt.resolvedUrl("./DashboardRepresentation.qml"), root) : null
    readonly property Kicker.DashboardWindow dashWindow: dashWindowComponent && dashWindowComponent.status === Component.Ready
        ? dashWindowComponent.createObject(root, { visualParent: root }) : null

    onWidthChanged: updateSizeHints()
    onHeightChanged: updateSizeHints()

    function updateSizeHints() {
        if (useCustomButtonImage) {
            if (vertical) {
                const scaledHeight = Math.floor(parent.width * (buttonIcon.implicitHeight / buttonIcon.implicitWidth));
                root.Layout.minimumHeight = scaledHeight;
                root.Layout.maximumHeight = scaledHeight;
                root.Layout.minimumWidth = PlasmaCore.Units.iconSizes.small;
                root.Layout.maximumWidth = inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1;
            } else {
                const scaledWidth = Math.floor(parent.height * (buttonIcon.implicitWidth / buttonIcon.implicitHeight));
                root.Layout.minimumWidth = scaledWidth;
                root.Layout.maximumWidth = scaledWidth;
                root.Layout.minimumHeight = PlasmaCore.Units.iconSizes.small;
                root.Layout.maximumHeight = inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1;
            }
        } else {
            root.Layout.minimumWidth = PlasmaCore.Units.iconSizes.small;
            root.Layout.maximumWidth = inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1;
            root.Layout.minimumHeight = PlasmaCore.Units.iconSizes.small;
            root.Layout.maximumHeight = inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1;
        }
    }

    Connections {
        target: PlasmaCore.Units.iconSizeHints

        function onPanelChanged() {
            root.updateSizeHints()
        }
    }

    PlasmaCore.IconItem {
        id: buttonIcon

        readonly property double aspectRatio: root.vertical
            ? implicitHeight / implicitWidth
            : implicitWidth / implicitHeight

        anchors.fill: parent

        active: mouseArea.containsMouse && !justOpenedTimer.running
        smooth: true
        source: root.useCustomButtonImage ? plasmoid.configuration.customButtonImage : plasmoid.configuration.icon

        // A custom icon could also be rectangular. However, if a square, custom, icon is given, assume it
        // to be an icon and round it to the nearest icon size again to avoid scaling artifacts.
        roundToIconSize: !root.useCustomButtonImage || aspectRatio === 1

        onSourceChanged: root.updateSizeHints()
    }

    MouseArea
    {
        id: mouseArea
        property bool wasExpanded: false;

        anchors.fill: parent

        hoverEnabled: !root.dashWindow || !root.dashWindow.visible

        onPressed: {
            if (!kicker.isDash) {
                wasExpanded = plasmoid.expanded
            }
        }

        onClicked: {
            if (kicker.isDash) {
                root.dashWindow.toggle();
                justOpenedTimer.start();
            } else {
                plasmoid.expanded = !wasExpanded;
            }
        }
    }

    Connections {
        target: plasmoid
        enabled: kicker.isDash && root.dashWindow !== null

        function onActivated() {
            root.dashWindow.toggle();
            justOpenedTimer.start();
        }
    }
}
