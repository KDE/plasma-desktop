/*
    SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
    SPDX-FileCopyrightText: 2015, 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.1
import Qt5Compat.GraphicalEffects

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.draganddrop 2.0 as DragDrop
import org.kde.plasma.private.trash 1.0 as TrashPrivate
import org.kde.kirigami 2.20 as Kirigami

import org.kde.kcmutils as KCM
import org.kde.config as KConfig

PlasmoidItem {
    id: root

    readonly property bool inPanel: (Plasmoid.location === PlasmaCore.Types.TopEdge
        || Plasmoid.location === PlasmaCore.Types.RightEdge
        || Plasmoid.location === PlasmaCore.Types.BottomEdge
        || Plasmoid.location === PlasmaCore.Types.LeftEdge)

    property bool containsAcceptableDrag: false

    Layout.minimumWidth: {
        if (inPanel) {
            return Plasmoid.formFactor === PlasmaCore.Types.Horizontal ? height : 1
        }
        return text.width
    }
    Layout.minimumHeight: {
        if (inPanel) {
            return Plasmoid.formFactor === PlasmaCore.Types.Vertical ? width : 1
        }
        return Kirigami.Units.iconSizes.small + text.height
    }

    Plasmoid.title: i18n("Trash")
    toolTipSubText: dirModel.count === 0
        ? i18n("Empty")
        : i18np("One item", "%1 items", dirModel.count)

    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground
    Plasmoid.icon: inPanel
        // In a panel: always be monochrome
        ? (dirModel.count > 0 ? "user-trash-full-symbolic" : "user-trash-symbolic")
        // On the desktop: whatevs
        : (dirModel.count > 0 ? "user-trash-full" : "user-trash")

    Plasmoid.onActivated: Qt.openUrlExternally("trash:/")

    Keys.onPressed: {
        switch (event.key) {
        case Qt.Key_Space:
        case Qt.Key_Enter:
        case Qt.Key_Return:
        case Qt.Key_Select:
            Plasmoid.activated();
            break;
        }
    }
    Accessible.name: Plasmoid.title
    Accessible.description: toolTipSubText
    Accessible.role: Accessible.Button

    DragDrop.DropArea {
        id: dropArea
        anchors.fill: parent
        preventStealing: true
        onDragEnter: containsAcceptableDrag = TrashPrivate.Trash.trashableUrls(event.mimeData.urls).length > 0
        onDragLeave: containsAcceptableDrag = false

        onDrop: {
            containsAcceptableDrag = false

            var trashableUrls = TrashPrivate.Trash.trashableUrls(event.mimeData.urls)
            if (trashableUrls.length > 0) {
                TrashPrivate.Trash.trashUrls(trashableUrls)
                event.accept(Qt.MoveAction)
            } else {
                event.accept(Qt.IgnoreAction) // prevent Plasma from spawning an applet
            }
        }
    }

    TrashPrivate.DirModel {
        id: dirModel
        url: "trash:/"
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            text: i18nc("a verb", "Open")
            icon.name: "document-open"
            onTriggered: Plasmoid.activated()
        },
        PlasmaCore.Action {
            text: i18nc("a verb", "Empty")
            icon.name: "trash-empty"
            enabled: dirModel.count > 0
            onTriggered: TrashPrivate.Trash.emptyTrash()
        },
        PlasmaCore.Action {
            text: i18n("Trash Settings…")
            icon.name: "configure"
            visible: KConfig.KAUthorized.authorizeControlModule("kcmtrash")
            onTriggered: KCM.KCMLauncher.open("kcmtrash")
        }
    ]

    Component.onCompleted: {
        Plasmoid.removeInternalAction("configure");
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        activeFocusOnTab: true
        hoverEnabled: true

        onClicked: Plasmoid.activated()
    }

    Kirigami.Icon {
        id: icon
        source: Plasmoid.icon
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: root.inPanel ? parent.bottom: text.top
        }
        active: mouseArea.containsMouse || dropArea.containsAcceptableDrag
    }

    DropShadow {
        id: textShadow

        anchors.fill: text

        visible: !root.inPanel

        horizontalOffset: 1
        verticalOffset: 1

        radius: 4
        samples: 9
        spread: 0.35

        color: "black"

        source: root.inPanel ? null : text
    }

    PC3.Label {
        id: text
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
        }
        width: Math.round(text.implicitWidth + Kirigami.Units.smallSpacing) // make sure label is not blurry
        text: Plasmoid.title + "\n" + root.toolTipSubText
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        visible: false // rendered by DropShadow
    }

    PlasmaCore.ToolTipArea {
        id: toolTip
        anchors.fill: parent
        mainText: Plasmoid.title
        subText:  root.toolTipSubText
    }
}
