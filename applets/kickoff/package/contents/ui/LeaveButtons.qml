/*
    SPDX-FileCopyrightText: 2020 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.12
import org.kde.plasma.private.kicker 0.1 as Kicker
import org.kde.plasma.components 2.0 as PlasmaComponents // for Menu + MenuItem
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core 2.0 as PlasmaCore
import QtQuick.Layouts 1.12

Item {
    id: leaveButtonRoot
    property alias leave: leaveButton

    RowLayout {
        id: leaveButtonsRow
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            right: leaveButton.left
            rightMargin: spacing
        }
        spacing: PlasmaCore.Units.smallSpacing * 2

        Repeater {
            model: systemFavorites

            PlasmaComponents3.ToolButton {
                // so that it lets the buttons elide...
                Layout.fillWidth: true
                // ... but does not make the buttons grow
                Layout.maximumWidth: implicitWidth
                text: model.display
                icon.name: model.decoration
                onClicked: {
                    systemFavorites.trigger(index, "", "")
                }
            }
        }

        Item { // compact layout
            Layout.fillWidth: true
        }
    }

    // Purely for layouting
    PlasmaComponents3.ToolButton {
        id: leaveButtonReference
        icon: leaveButton.icon
        text: leaveButton.text
        visible: false
    }

    PlasmaComponents3.ToolButton {
        id: leaveButton

        readonly property int currentId: plasmoid.configuration.primaryActions

        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
        }

        display: {
            if (leaveButtonsRow.implicitWidth > leaveButtonRoot.width - leaveButtonsRow.anchors.rightMargin - leaveButtonReference.width) {
                return PlasmaComponents3.AbstractButton.IconOnly;
            }
            return PlasmaComponents3.AbstractButton.TextBesideIcon;
        }

        icon.name: {
            return [
                "system-log-out",
                "system-shutdown",
                "view-more-symbolic"
            ][currentId];
        }
        text: {
            return [
                i18n("Leave…"),
                i18n("Power…"),
                i18n("More…")
            ][currentId];
        }

        // Make it look pressed while the menu is open
        checked: contextMenu.status === PlasmaComponents.DialogStatus.Open
        onPressed: contextMenu.openRelative()

        Keys.forwardTo: [leaveButtonRoot]

        PlasmaComponents3.ToolTip {
            text: {
                return [
                    i18n("Leave"),
                    i18n("Power"),
                    i18n("More")
                ][leaveButton.currentId];
            }
            visible: leaveButton.display === PlasmaComponents3.AbstractButton.IconOnly && leaveButton.hovered
        }
    }

    Instantiator {
        model: Kicker.SystemModel {
            id: systemModel
            favoritesModel: globalFavorites
        }
        delegate: PlasmaComponents.MenuItem {
            text: model.display
            icon: model.decoration
            visible: !String(plasmoid.configuration.systemFavorites).includes(model.favoriteId)

            onClicked: systemModel.trigger(index, "", "")
        }

        onObjectAdded: {
            contextMenu.addMenuItem(object);
        }
    }

    PlasmaComponents.Menu {
        id: contextMenu
        visualParent: leaveButton
        placement: {
            switch (plasmoid.location) {
            case PlasmaCore.Types.LeftEdge:
            case PlasmaCore.Types.RightEdge:
            case PlasmaCore.Types.TopEdge:
                return PlasmaCore.Types.BottomPosedRightAlignedPopup;
            case PlasmaCore.Types.BottomEdge:
            default:
                return PlasmaCore.Types.TopPosedRightAlignedPopup;
            }
        }
    }

    Keys.onPressed: {
        if (event.key == Qt.Key_Tab && mainTabGroup.state == "top") {
            keyboardNavigation.state = "LeftColumn"
            root.currentView.forceActiveFocus()
            event.accepted = true;
        }
    }

}
