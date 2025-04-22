/*
    SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
    SPDX-FileCopyrightText: 2015, 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2023 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras
import org.kde.draganddrop 2.0 as DragDrop
import org.kde.plasma.private.trash as TrashPrivate
import org.kde.kirigami 2.20 as Kirigami

import org.kde.kcmutils as KCM
import org.kde.config as KConfig

PlasmoidItem {
    id: root

    readonly property bool inPanel: (Plasmoid.location === PlasmaCore.Types.TopEdge
        || Plasmoid.location === PlasmaCore.Types.RightEdge
        || Plasmoid.location === PlasmaCore.Types.BottomEdge
        || Plasmoid.location === PlasmaCore.Types.LeftEdge)
    readonly property bool hasContents: dirModel.count > 0

    property bool containsAcceptableDrag: false

    Plasmoid.title: i18nc("@title the name of the Trash widget", "Trash")
    toolTipSubText: hasContents
        ? i18ncp("@info:status The trash contains this many items in it", "One item", "%1 items", dirModel.count)
        : i18nc("@info:status The trash is empty", "Empty")

    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground
    Plasmoid.icon: {
        let iconName = (hasContents ? "user-trash-full" : "user-trash");

        if (inPanel) {
            return iconName += "-symbolic";
        }

        return iconName;
    }
    Plasmoid.status: hasContents ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus

    Plasmoid.onActivated: TrashPrivate.Trash.openTrash()

    Keys.onPressed: event => {
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

    TrashPrivate.DirModel {
        id: dirModel
        url: "trash:/"
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            text: i18nc("@action:inmenu Open the trash", "Open")
            icon.name: "document-open-symbolic"
            onTriggered: Plasmoid.activated()
        },
        PlasmaCore.Action {
            text: i18nc("@action:inmenu Empty the trash", "Empty")
            icon.name: "trash-empty-symbolic"
            enabled: hasContents
            onTriggered: TrashPrivate.Trash.emptyTrash()
        },
        PlasmaCore.Action {
            text: i18nc("@action:inmenu", "Trash Settings…")
            icon.name: "configure-symbolic"
            visible: KConfig.KAuthorized.authorizeControlModule("kcm_trash")
            onTriggered: KCM.KCMLauncher.open("kcm_trash")
        }
    ]

    Component.onCompleted: {
        Plasmoid.removeInternalAction("configure");
    }

    // Only exists because the default CompactRepresentation doesn't:
    // - allow defining a custom drop handler
    // - expose the ability to show text below or beside the icon
    // TODO remove once it gains those features
    preferredRepresentation: fullRepresentation
    fullRepresentation: MouseArea {
        id: mouseArea

        activeFocusOnTab: true
        hoverEnabled: true

        onClicked: Plasmoid.activated()

        DragDrop.DropArea {
            anchors.fill: parent
            preventStealing: true
            onDragEnter: root.containsAcceptableDrag = TrashPrivate.Trash.trashableUrls(event.mimeData.urls).length > 0
            onDragLeave: root.containsAcceptableDrag = false

            onDrop: {
                root.containsAcceptableDrag = false

                var trashableUrls = TrashPrivate.Trash.trashableUrls(event.mimeData.urls)
                if (trashableUrls.length > 0) {
                    TrashPrivate.Trash.trashUrls(trashableUrls)
                    event.accept(Qt.MoveAction)
                } else {
                    event.accept(Qt.IgnoreAction) // prevent Plasma from spawning an applet
                }
            }
        }

        Kirigami.Icon {
            source: Plasmoid.icon
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                bottom: root.inPanel ? parent.bottom: text.top
            }
            active: mouseArea.containsMouse || root.containsAcceptableDrag
        }

        PlasmaExtras.ShadowedLabel {
            id: text
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
            }
            width: Math.round(text.implicitWidth + Kirigami.Units.smallSpacing) // make sure label is not blurry
            text: Plasmoid.title + "\n" + root.toolTipSubText
            horizontalAlignment: Text.AlignHCenter
            visible: !root.inPanel
        }

        PlasmaCore.ToolTipArea {
            anchors.fill: parent
            mainText: Plasmoid.title
            subText:  root.toolTipSubText
        }
    }
}
