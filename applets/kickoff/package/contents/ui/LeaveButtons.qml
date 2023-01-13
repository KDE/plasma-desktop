/*
    SPDX-FileCopyrightText: 2020 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.plasma.private.kicker 0.1 as Kicker
import org.kde.plasma.components 2.0 as PC2 // for Menu + MenuItem
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.16 as Kirigami

RowLayout {
    id: root
    readonly property alias buttonImplicitWidth: buttonRepeaterRow.implicitWidth
    property alias leave: leaveButton
    property bool shouldCollapseButtons: false
    spacing: plasmoid.rootItem.backgroundMetrics.spacing

    Kicker.SystemModel {
        id: systemModel
        favoritesModel: plasmoid.rootItem.rootModel.systemFavoritesModel
    }

    Item {
        Layout.fillWidth: !plasmoid.configuration.showActionButtonCaptions && plasmoid.configuration.primaryActions === 3
    }

    RowLayout {
        id: buttonRepeaterRow
        // HACK Can't use visible as buttons are invisible after switching from Power and session to Power
        enabled: !root.shouldCollapseButtons
        opacity: enabled ? 1 : 0
        spacing: parent.spacing
        Repeater {
            id: buttonRepeater
            model: systemModel
            delegate: PC3.ToolButton {
                id: buttonDelegate
                text: model.display
                icon.name: model.decoration
                // TODO: Don't generate items that will never be seen. Maybe DelegateModel can help?
                visible: String(plasmoid.configuration.systemFavorites).includes(model.favoriteId)
                onClicked: systemModel.trigger(index, "", null)
                display: plasmoid.configuration.showActionButtonCaptions ? PC3.AbstractButton.TextBesideIcon : PC3.AbstractButton.IconOnly;
                Layout.rightMargin: model.favoriteId === "switch-user" && plasmoid.configuration.primaryActions === 3 ? PlasmaCore.Units.gridUnit : undefined

                PC3.ToolTip.text: text
                PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
                PC3.ToolTip.visible: display === PC3.AbstractButton.IconOnly && hovered

                Keys.onLeftPressed: if (!LayoutMirroring.enabled) {
                    nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
                } else if (index < buttonRepeater.count - 1 || leaveButton.visible) {
                    nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
                }
                Keys.onRightPressed: if (LayoutMirroring.enabled) {
                    nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
                } else if (index < buttonRepeater.count - 1 || leaveButton.visible) {
                    nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
                }
            }
        }
    }

    Item {
        Layout.fillWidth: !plasmoid.configuration.showActionButtonCaptions || plasmoid.configuration.primaryActions !== 3
    }

    PC3.ToolButton {
        id: leaveButton
        readonly property int currentId: plasmoid.configuration.primaryActions
        Accessible.role: Accessible.ButtonMenu
        icon.width: PlasmaCore.Units.iconSizes.smallMedium
        icon.height: PlasmaCore.Units.iconSizes.smallMedium
        icon.name: ["system-log-out", "system-shutdown", "view-more-symbolic", "view-more-symbolic"][currentId]
        display: root.shouldCollapseButtons ? PC3.AbstractButton.TextBesideIcon : PC3.AbstractButton.IconOnly
        text: [i18n("Leave"), i18n("Power"), i18n("More"), i18n("More")][currentId]
        visible: plasmoid.configuration.primaryActions !== 3 || root.shouldCollapseButtons
        // Make it look pressed while the menu is open
        down: contextMenu.status === PC2.DialogStatus.Open || pressed
        PC3.ToolTip.text: text
        PC3.ToolTip.visible: hovered
        PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
        Keys.onLeftPressed: if (!LayoutMirroring.enabled) {
            nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
        }
        Keys.onRightPressed: if (LayoutMirroring.enabled) {
            nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
        }
        onPressed: contextMenu.openRelative()
    }

    Instantiator {
        model: systemModel
        // Not a QQC1 MenuItem. It's actually a custom QQuickItem.
        delegate: PC2.MenuItem {
            text: model.display
            icon: model.decoration
            // TODO: Don't generate items that will never be seen. Maybe DelegateModel can help?
            visible: !String(plasmoid.configuration.systemFavorites).includes(model.favoriteId) || root.shouldCollapseButtons
            Accessible.role: Accessible.MenuItem
            onClicked: systemModel.trigger(index, "", null)
        }
        onObjectAdded: contextMenu.addMenuItem(object)
        onObjectRemoved: contextMenu.removeMenuItem(object)
    }

    // Not a QQC1 Menu. It's actually a custom QObject that uses a QMenu.
    PC2.Menu {
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
}
