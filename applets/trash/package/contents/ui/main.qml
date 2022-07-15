/*
    SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
    SPDX-FileCopyrightText: 2015, 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.kcoreaddons 1.0 as KCoreAddons
import org.kde.kquickcontrolsaddons 2.0
import org.kde.draganddrop 2.0 as DragDrop
import org.kde.plasma.private.trash 1.0 as TrashPrivate

DragDrop.DropArea {
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
        return PlasmaCore.Units.iconSizes.small + text.height
    }

    Layout.maximumWidth: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1
    Layout.maximumHeight: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1

    readonly property int formFactor: plasmoid.formFactor
    readonly property bool constrained: formFactor === PlasmaCore.Types.Vertical || formFactor === PlasmaCore.Types.Horizontal

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground
    Plasmoid.icon: (dirModel.count > 0) ? "user-trash-full": "user-trash"
    Plasmoid.onActivated: action_open()

    preventStealing: true

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

    TrashPrivate.DirModel {
        id: dirModel
        url: "trash:/"
    }

    function action_open() {
        Qt.openUrlExternally("trash:/");
    }

    function action_empty() {
        TrashPrivate.Trash.emptyTrash()
    }

    function action_openkcm() {
        KCMShell.open("kcmtrash");
    }

    Component.onCompleted: {
        plasmoid.removeAction("configure");
        plasmoid.setAction("open", i18nc("a verb", "Open"),"document-open");
        plasmoid.setAction("empty",i18nc("a verb", "Empty"),"trash-empty");
        plasmoid.action("empty").enabled = Qt.binding(function() {
            return dirModel.count > 0;
        });

        if (KCMShell.authorize("kcmtrash.desktop").length > 0) {
            plasmoid.setAction("openkcm", i18n("Trash Settings…"), "configure");
        }
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
        active: toolTip.containsMouse || root.containsAcceptableDrag
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

    PC3.Label {
        id: text
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
        }
        width: Math.round(text.implicitWidth + PlasmaCore.Units.smallSpacing) // make sure label is not blurry
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
