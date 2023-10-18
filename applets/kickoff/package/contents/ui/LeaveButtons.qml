/*
    SPDX-FileCopyrightText: 2020 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.plasma.private.kicker 0.1 as Kicker
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kitemmodels as KItemModels
import org.kde.plasma.plasmoid 2.0

RowLayout {
    id: root
    readonly property alias buttonImplicitWidth: buttonRepeaterRow.implicitWidth
    property bool shouldCollapseButtons: false
    spacing: kickoff.backgroundMetrics.spacing

    Kicker.SystemModel {
        id: systemModel
        favoritesModel: kickoff.rootModel.systemFavoritesModel
    }

    component FilteredModel : KItemModels.KSortFilterProxyModel {
        sourceModel: systemModel

        function systemFavoritesContainsRow(sourceRow, sourceParent) {
            const FavoriteIdRole = sourceModel.KItemModels.KRoleNames.role("favoriteId");
            const favoriteId = sourceModel.data(sourceModel.index(sourceRow, 0, sourceParent), FavoriteIdRole);
            return String(Plasmoid.configuration.systemFavorites).includes(favoriteId);
        }

        function trigger(index) {
            const sourceIndex = mapToSource(this.index(index, 0));
            systemModel.trigger(sourceIndex.row, "", null);
        }

        Component.onCompleted: {
            Plasmoid.configuration.valueChanged.connect((key, value) => {
                if (key === "systemFavorites") {
                    invalidateFilter();
                }
            });
        }
    }

    FilteredModel {
        id: filteredButtonsModel
        filterRowCallback: (sourceRow, sourceParent) =>
            systemFavoritesContainsRow(sourceRow, sourceParent)
    }

    FilteredModel {
        id: filteredMenuItemsModel
        filterRowCallback: root.shouldCollapseButtons
            ? null /*i.e. keep all rows*/
            : (sourceRow, sourceParent) => !systemFavoritesContainsRow(sourceRow, sourceParent)
    }

    Item {
        Layout.fillWidth: !Plasmoid.configuration.showActionButtonCaptions && Plasmoid.configuration.primaryActions === 3
    }

    RowLayout {
        id: buttonRepeaterRow
        // HACK Can't use `visible` property, as the layout needs to be
        // visible to be able to update its implicit size, which in turn is
        // be used to set shouldCollapseButtons.
        enabled: !root.shouldCollapseButtons
        opacity: !root.shouldCollapseButtons ? 1 : 0
        spacing: parent.spacing
        Repeater {
            id: buttonRepeater

            model: filteredButtonsModel
            delegate: PC3.ToolButton {
                text: model.display
                icon.name: model.decoration
                onClicked: filteredButtonsModel.trigger(index);
                display: Plasmoid.configuration.showActionButtonCaptions ? PC3.AbstractButton.TextBesideIcon : PC3.AbstractButton.IconOnly;
                Layout.rightMargin: model.favoriteId === "switch-user" && Plasmoid.configuration.primaryActions === 3 ? Kirigami.Units.gridUnit : undefined

                PC3.ToolTip.text: text
                PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
                PC3.ToolTip.visible: display === PC3.AbstractButton.IconOnly && hovered

                Keys.onTabPressed: event => {
                    if (index === buttonRepeater.count - 1 && !leaveButton.shouldBeVisible) {
                        kickoff.firstHeaderItem.forceActiveFocus(Qt.TabFocusReason)
                    } else {
                        event.accepted = false
                    }
                }
                Keys.onLeftPressed: event => {
                    if (Qt.application.layoutDirection === Qt.LeftToRight) {
                        nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
                    } else if (index < buttonRepeater.count - 1 || leaveButton.shouldBeVisible) {
                        nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
                    }
                }
                Keys.onRightPressed: event => {
                    if (Qt.application.layoutDirection === Qt.RightToLeft) {
                        nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
                    } else if (index < buttonRepeater.count - 1 || leaveButton.shouldBeVisible) {
                        nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
                    }
                }
            }
        }
    }

    Item {
        Layout.fillWidth: !Plasmoid.configuration.showActionButtonCaptions || Plasmoid.configuration.primaryActions !== 3
    }

    PC3.ToolButton {
        id: leaveButton
        readonly property int currentId: Plasmoid.configuration.primaryActions
        readonly property bool shouldBeVisible: Plasmoid.configuration.primaryActions !== 3 || root.shouldCollapseButtons
        Accessible.role: Accessible.ButtonMenu
        icon.width: Kirigami.Units.iconSizes.smallMedium
        icon.height: Kirigami.Units.iconSizes.smallMedium
        icon.name: ["system-log-out", "system-shutdown", "view-more-symbolic", "view-more-symbolic"][currentId]
        display: root.shouldCollapseButtons ? PC3.AbstractButton.TextBesideIcon : PC3.AbstractButton.IconOnly
        text: [i18n("Leave"), i18n("Power"), i18n("More"), i18n("More")][currentId]
        visible: shouldBeVisible
        // Make it look pressed while the menu is open
        down: contextMenu.status === PlasmaExtras.Menu.Open || pressed
        PC3.ToolTip.text: text
        PC3.ToolTip.visible: hovered
        PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
        Keys.onTabPressed: event => {
            kickoff.firstHeaderItem.forceActiveFocus(Qt.TabFocusReason);
        }
        Keys.onLeftPressed: event => {
            if (Qt.application.layoutDirection == Qt.LeftToRight) {
                nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
            }
        }
        Keys.onRightPressed: event => {
            if (Qt.application.layoutDirection == Qt.RightToLeft) {
                nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
            }
        }
        onPressed: contextMenu.openRelative()
    }

    Instantiator {
        model: filteredMenuItemsModel
        delegate: PlasmaExtras.MenuItem {
            text: model.display
            icon: model.decoration
            onClicked: filteredMenuItemsModel.trigger(index)
        }
        onObjectAdded: (index, object) => contextMenu.addMenuItem(object)
        onObjectRemoved: (index, object) => contextMenu.removeMenuItem(object)
    }

    PlasmaExtras.Menu {
        id: contextMenu
        visualParent: leaveButton
        placement: {
            switch (Plasmoid.location) {
            case PlasmaCore.Types.LeftEdge:
            case PlasmaCore.Types.RightEdge:
            case PlasmaCore.Types.TopEdge:
                return PlasmaExtras.Menu.BottomPosedRightAlignedPopup;
            case PlasmaCore.Types.BottomEdge:
            default:
                return PlasmaExtras.Menu.TopPosedRightAlignedPopup;
            }
        }
    }
}
