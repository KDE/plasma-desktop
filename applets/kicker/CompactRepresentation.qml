/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import org.kde.plasma.private.kicker as Kicker

Item {
    id: root

    readonly property bool isDash: Plasmoid.pluginName === "org.kde.plasma.kickerdash"
    readonly property bool vertical: (Plasmoid.formFactor === PlasmaCore.Types.Vertical)
    readonly property bool useCustomButtonImage: (Plasmoid.configuration.useCustomButtonImage
        && Plasmoid.configuration.customButtonImage.length !== 0)

    property string toolTipSubText: ""
    property Component dashWindowComponent
    readonly property Kicker.DashboardWindow dashWindow: isDash && dashWindowComponent && dashWindowComponent.status === Component.Ready
        ? dashWindowComponent.createObject(root, { visualParent: root }) as Kicker.DashboardWindow : null

    onWidthChanged: updateSizeHints()
    onHeightChanged: updateSizeHints()

    function updateSizeHints(): void {
        if (useCustomButtonImage && imageFallback.visible) {
            if (vertical) {
                const scaledHeight = Math.floor(parent?.width * (imageFallback.implicitHeight / imageFallback.implicitWidth));
                root.Layout.minimumWidth = -1;
                root.Layout.minimumHeight = scaledHeight;
                root.Layout.maximumWidth = Kirigami.Units.iconSizes.huge;
                root.Layout.maximumHeight = scaledHeight;
            } else {
                const scaledWidth = Math.floor(parent?.height * (imageFallback.implicitWidth / imageFallback.implicitHeight));
                root.Layout.minimumWidth = scaledWidth;
                root.Layout.minimumHeight = -1;
                root.Layout.maximumWidth = scaledWidth;
                root.Layout.maximumHeight = Kirigami.Units.iconSizes.huge;
            }
        } else {
            root.Layout.minimumWidth = -1;
            root.Layout.minimumHeight = -1;
            root.Layout.maximumWidth = Kirigami.Units.iconSizes.huge;
            root.Layout.maximumHeight = Kirigami.Units.iconSizes.huge;
        }
    }

    Kirigami.Icon {
        id: buttonIcon

        anchors.fill: parent

        active: mouseArea.containsMouse
        source: root.useCustomButtonImage ? Plasmoid.configuration.customButtonImage : Plasmoid.configuration.icon
        visible: !imageFallback.visible

        // A custom icon could also be rectangular, but Kirigami.Icon only supports square ones.
        // If a square, custom, icon is given, we load it using Kirigami.Icon and round it to
        // the nearest icon size again to avoid scaling artifacts.
        roundToIconSize: true

        onSourceChanged: root.updateSizeHints()
        onVisibleChanged: root.updateSizeHints()
    }

    Image {
        id: imageFallback

        readonly property double aspectRatio: root.vertical
            ? implicitHeight / implicitWidth
            : implicitWidth / implicitHeight

        anchors.fill: parent

        visible: root.useCustomButtonImage && status == Image.Ready && aspectRatio !== 1 && source !== ""

        source: Plasmoid.icon.startsWith("/") ? "file:" + Plasmoid.icon.split("/").map(encodeURIComponent).join("/") : ""
        onSourceChanged: root.updateSizeHints()
        onVisibleChanged: root.updateSizeHints()

        fillMode: Image.PreserveAspectFit
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        property bool wasExpanded: false;

        activeFocusOnTab: true
        hoverEnabled: !root.dashWindow || !root.dashWindow.visible

        Keys.onReturnPressed: Plasmoid.activated()
        Keys.onEnterPressed: Plasmoid.activated()
        Keys.onSpacePressed: Plasmoid.activated()
        Keys.onSelectPressed: Plasmoid.activated()

        Accessible.name: Plasmoid.title
        Accessible.description: root.toolTipSubText
        Accessible.role: Accessible.Button

        onPressed: mouse => {
            if (!root.isDash) {
                wasExpanded = kicker.expanded
            }
        }

        onClicked: mouse => {
            if (root.isDash) {
                root.dashWindow.toggle();
            } else {
                kicker.expanded = !wasExpanded;
            }
        }
    }

    Connections {
        target: Plasmoid
        enabled: root.isDash && root.dashWindow !== null

        function onActivated(): void {
            root.dashWindow.toggle();
        }
    }
}
