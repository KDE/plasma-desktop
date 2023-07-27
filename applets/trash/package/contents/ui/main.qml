/*
    SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
    SPDX-FileCopyrightText: 2015, 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Controls as QQC2
import QtQuick.Layouts 1.1
import Qt5Compat.GraphicalEffects

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.draganddrop 2.0 as DragDrop
import org.kde.plasma.private.trash 1.0 as TrashPrivate
import org.kde.kirigami 2.20 as Kirigami

import org.kde.kcmutils as KCM
import org.kde.config as KConfig

PlasmoidItem {
    id: root

    readonly property bool inPanel: (plasmoid.location === PlasmaCore.Types.TopEdge
        || plasmoid.location === PlasmaCore.Types.RightEdge
        || plasmoid.location === PlasmaCore.Types.BottomEdge
        || plasmoid.location === PlasmaCore.Types.LeftEdge)

    property bool containsAcceptableDrag: false

    Layout.minimumWidth: {
        if (constrained) {
            return formFactor === PlasmaCore.Types.Horizontal ? height : 1
        }
        return text.width
    }
    Layout.minimumHeight: {
        if (constrained) {
            return formFactor === PlasmaCore.Types.Vertical ? width : 1
        }
        return Kirigami.Units.iconSizes.small + text.height
    }

    readonly property int formFactor: plasmoid.formFactor
    readonly property bool constrained: formFactor === PlasmaCore.Types.Vertical || formFactor === PlasmaCore.Types.Horizontal

    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground
    Plasmoid.icon: (dirModel.count > 0) ? "user-trash-full": "user-trash"
    Plasmoid.onActivated: openTrash()

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
    Accessible.name: toolTip.mainText
    Accessible.description: toolTip.subText
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

    function openTrash() {
        Qt.openUrlExternally("trash:/");
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            text: i18nc("a verb", "Open")
            icon.name: "document-open"
            onTriggered: Qt.openUrlExternally("trash:/")
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
        plasmoid.removeInternalAction("configure");
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        activeFocusOnTab: true

        onClicked: Qt.openUrlExternally("trash:/");
    }

    PlasmaCore.IconItem {
        id: icon
        source: plasmoid.icon
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: constrained ? parent.bottom: text.top
        }
        active: toolTip.containsMouse || dropArea.containsAcceptableDrag
    }

    DropShadow {
        id: textShadow

        anchors.fill: text

        visible: !constrained

        horizontalOffset: 1
        verticalOffset: 1

        radius: 4
        samples: 9
        spread: 0.35

        color: "black"

        source: constrained ? null : text
    }

    QQC2.Label {
        id: text
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
        }
        width: Math.round(text.implicitWidth + Kirigami.Units.smallSpacing) // make sure label is not blurry
        text: (dirModel.count === 0) ? i18n("Trash\nEmpty") : i18np("Trash\nOne item", "Trash\n %1 items", dirModel.count)
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        visible: false // rendered by DropShadow
    }

    PlasmaCore.ToolTipArea {
        id: toolTip
        anchors.fill: parent
        mainText: i18n("Trash")
        subText: (dirModel.count === 0) ? i18n("Empty") : i18np("One item", "%1 items", dirModel.count)
    }
}
