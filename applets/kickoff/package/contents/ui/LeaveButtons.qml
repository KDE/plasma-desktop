/*
    SPDX-FileCopyrightText: 2020 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.plasma.private.kicker as Kicker
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.components as PC3
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KItemModels
import org.kde.plasma.plasmoid

RowLayout {
    id: root

    required property real maximumWidth

    // When all actions are configured as primary, none of them should be
    // hidden into the overflow menu as long as the available space allows
    // it.
    readonly property var __layout: {
        const allActionsArePrimary = Plasmoid.configuration.primaryActions === 3;
        const actionButtonsImplicitWidth = buttonsRepeaterRow.implicitWidth;

        let configuredImplicitWidth_IconOnly = actionButtonsImplicitWidth;
        let configuredImplicitWidth_TextBesideIcon = actionButtonsImplicitWidth;

        if (!allActionsArePrimary) {
            configuredImplicitWidth_IconOnly += overflowMenuButtonIconOnly.implicitWidth + spacing;
            configuredImplicitWidth_TextBesideIcon += overflowMenuButtonTextBesideIcon.implicitWidth + spacing;
        }

        const collapseActionButtons = configuredImplicitWidth_IconOnly > maximumWidth;
        const collapseOverflowMenuButton = !collapseActionButtons && configuredImplicitWidth_TextBesideIcon > maximumWidth;

        // Can't rely on the transient Item::visible property
        const overflowMenuButtonIsVisible = !allActionsArePrimary || collapseActionButtons;

        return {
            allActionsArePrimary,
            collapseActionButtons,
            collapseOverflowMenuButton,
            overflowMenuButtonIsVisible,
        };
    }

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
        filterRowCallback: root.__layout.collapseActionButtons
            ? null /*i.e. keep all rows*/
            : (sourceRow, sourceParent) => !systemFavoritesContainsRow(sourceRow, sourceParent)
    }

    Item {
        Layout.fillWidth: !Plasmoid.configuration.showActionButtonCaptions && root.__layout.allActionsArePrimary
    }

    RowLayout {
        id: buttonsRepeaterRow
        // HACK Can't use `visible` property, as the layout needs to be
        // visible to be able to update its implicit size, which in turn is
        // be used to set collapseActionButtons.
        enabled: !root.__layout.collapseActionButtons
        opacity: !root.__layout.collapseActionButtons ? 1 : 0
        spacing: parent.spacing
        Repeater {
            id: buttonRepeater

            model: filteredButtonsModel
            delegate: PC3.ToolButton {
                required property int index
                required property var model

                text: model.display
                icon.name: model.decoration
                onClicked: {
                    filteredButtonsModel.trigger(index);
                    if (kickoff.hideOnWindowDeactivate) {
                        kickoff.expanded = false;
                    }
                }
                display: Plasmoid.configuration.showActionButtonCaptions ? PC3.AbstractButton.TextBesideIcon : PC3.AbstractButton.IconOnly;
                Layout.rightMargin: model.favoriteId === "switch-user" && root.__layout.allActionsArePrimary ? Kirigami.Units.gridUnit : undefined

                PC3.ToolTip.text: text
                PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
                PC3.ToolTip.visible: display === PC3.AbstractButton.IconOnly && hovered

                Keys.onTabPressed: event => {
                    if (index === buttonRepeater.count - 1 && !root.__layout.overflowMenuButtonIsVisible) {
                        kickoff.firstHeaderItem.forceActiveFocus(Qt.TabFocusReason)
                    } else {
                        event.accepted = false
                    }
                }
                Keys.onLeftPressed: event => {
                    if (Qt.application.layoutDirection === Qt.LeftToRight) {
                        nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
                    } else if (index < buttonRepeater.count - 1 || root.__layout.overflowMenuButtonIsVisible) {
                        nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
                    }
                }
                Keys.onRightPressed: event => {
                    if (Qt.application.layoutDirection === Qt.RightToLeft) {
                        nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
                    } else if (index < buttonRepeater.count - 1 || root.__layout.overflowMenuButtonIsVisible) {
                        nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
                    }
                }
            }
        }
    }

    Item {
        Layout.fillWidth: !Plasmoid.configuration.showActionButtonCaptions || !root.__layout.allActionsArePrimary
    }

    // Just like Kirigami.ActionToolBar, it takes two actual instances of a
    // button with different display modes to calculate the layout properly
    // without binding loops.
    component OverflowMenuButton : PC3.ToolButton {
        Accessible.role: Accessible.ButtonMenu
        icon.width: Kirigami.Units.iconSizes.smallMedium
        icon.height: Kirigami.Units.iconSizes.smallMedium
        icon.name: ["system-log-out", "system-shutdown", "view-more-symbolic", "view-more-symbolic"][Plasmoid.configuration.primaryActions]
        text: [i18n("Session"), i18n("Power"), i18n("More"), i18n("Session and Power ")][Plasmoid.configuration.primaryActions]
        // Make it look pressed while the menu is open
        down: contextMenu.status === PlasmaExtras.Menu.Open || pressed
        PC3.ToolTip.text: text
        PC3.ToolTip.visible: hovered
        PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
        Keys.onTabPressed: event => {
            kickoff.firstHeaderItem.forceActiveFocus(Qt.TabFocusReason);
        }
        Keys.onLeftPressed: event => {
            if (!mirrored) {
                nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
            }
        }
        Keys.onRightPressed: event => {
            if (mirrored) {
                nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
            }
        }
        onPressed: {
            contextMenu.visualParent = this;
            contextMenu.openRelative();
        }
    }

    OverflowMenuButton {
        id: overflowMenuButtonTextBesideIcon
        display: PC3.AbstractButton.TextBesideIcon
        visible: root.__layout.overflowMenuButtonIsVisible && !root.__layout.collapseOverflowMenuButton
    }

    OverflowMenuButton {
        id: overflowMenuButtonIconOnly
        display: PC3.AbstractButton.IconOnly
        visible: root.__layout.overflowMenuButtonIsVisible && root.__layout.collapseOverflowMenuButton
    }

    Instantiator {
        model: filteredMenuItemsModel
        delegate: PlasmaExtras.MenuItem {
            required property int index
            required property var model

            text: model.display
            icon: model.decoration
            onClicked: filteredMenuItemsModel.trigger(index)
        }
        onObjectAdded: (index, object) => contextMenu.addMenuItem(object)
        onObjectRemoved: (index, object) => contextMenu.removeMenuItem(object)
    }

    PlasmaExtras.Menu {
        id: contextMenu
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
