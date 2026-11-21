/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
    SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM
import org.kde.config as KConfig
import org.kde.plasma.plasmoid
import org.kde.plasma.private.kicker as Kicker

KCM.SimpleKCM {
    id: root

    property string cfg_menuLabel: menuLabel.text
    property alias cfg_icon: iconButton.iconName
    property alias cfg_appNameFormat: appNameFormat.currentIndex
    property bool cfg_paneSwap: Plasmoid.configuration.paneSwap
    property int cfg_favoritesDisplay: Plasmoid.configuration.favoritesDisplay
    property int cfg_applicationsDisplay: Plasmoid.configuration.applicationsDisplay
    property alias cfg_alphaSort: alphaSort.checked
    property var cfg_systemFavorites: String(Plasmoid.configuration.systemFavorites)
    property int cfg_primaryActions: Plasmoid.configuration.primaryActions
    property alias cfg_showActionButtonCaptions: showActionButtonCaptions.checked
    property alias cfg_compactMode: compactModeCheckbox.checked
    property alias cfg_highlightNewlyInstalledApps: highlightNewlyInstalledAppsCheckbox.checked
    property alias cfg_switchCategoryOnHover: switchCategoryOnHoverCheckbox.checked

    Kirigami.FormLayout {
        Kicker.IconButton {
            id: iconButton
            Kirigami.FormData.label: i18nc("@label prefix for icon-only button", "Icon:")
            hasLabelText: menuLabel.text != ""
            widgetName: i18nc("@label match translation with Kickoff's metadata.json", "Application Launcher")
        }

        Kirigami.ActionTextField {
            id: menuLabel
            enabled: Plasmoid.formFactor !== PlasmaCore.Types.Vertical
            Kirigami.FormData.label: i18nc("@label:textbox", "Text label:")
            text: Plasmoid.configuration.menuLabel
            placeholderText: i18nc("@info:placeholder", "Type here to add a text label")
            onTextEdited: {
                root.cfg_menuLabel = menuLabel.text

                // This is to make sure that we always have a icon if there is no text.
                // If the user remove the icon and remove the text, without this, we'll have no icon and no text.
                // This is to force the icon to be there.
                if (!menuLabel.text) {
                    root.cfg_icon = root.cfg_icon || Kicker.Tools.defaultIconName
                }
            }
            rightActions: QQC2.Action {
                icon.name: "edit-clear"
                enabled: menuLabel.text !== ""
                text: i18nc("@action:button", "Reset menu label")
                onTriggered: {
                    menuLabel.clear()
                    root.cfg_menuLabel = ""
                    root.cfg_icon = root.cfg_icon || Kicker.Tools.defaultIconName
                }
            }
        }

        QQC2.Label {
            Layout.fillWidth: true
            Layout.maximumWidth: Kirigami.Units.gridUnit * 25
            visible: Plasmoid.formFactor === PlasmaCore.Types.Vertical
            text: i18nc("@info", "A text label cannot be set when the Panel is vertical.")
            wrapMode: Text.Wrap
            font: Kirigami.Theme.smallFont
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.ComboBox {
            id: appNameFormat

            Kirigami.FormData.label: i18nc("@label:listbox", "Show applications as:")

            model: [i18nc("@item:inlistbox", "Name only"), i18nc("@item:inlistbox", "Description only"), i18nc("@item:inlistbox", "Name (Description)"), i18nc("@item:inlistbox", "Description (Name)")]
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        RowLayout {
            Kirigami.FormData.label: i18nc("General options", "General:")
            spacing: Kirigami.Units.smallSpacing
            QQC2.CheckBox {
                id: alphaSort
                text: i18nc("@option:check", "Sort applications alphabetically")
            }

            Kirigami.ContextualHelpButton {
                toolTipText: i18nc("@info:whatsthis", "This doesn't affect how applications are sorted in either search results or the favorites page.")
            }
        }

        QQC2.CheckBox {
            id: compactModeCheckbox
            text: i18nc("@option:check", "Use compact list item style")
            checked: Plasmoid.configuration.compactMode
        }

        QQC2.CheckBox {
            id: highlightNewlyInstalledAppsCheckbox
            text: i18nc("@option:check", "Highlight newly-installed applications")
        }

        QQC2.CheckBox {
            id: switchCategoryOnHoverCheckbox
            text: i18nc("@option:check", "Switch sidebar categories when hovering over them")
        }

        QQC2.Button {
            enabled: KConfig.KAuthorized.authorizeControlModule("kcm_plasmasearch")
            icon.name: "settings-configure"
            text: i18nc("@action:button opens plasmasearch kcm", "Configure Search Plugins…")
            onClicked: KCM.KCMLauncher.openSystemSettings("kcm_plasmasearch")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.RadioButton {
            id: paneSwapOff
            Kirigami.FormData.label: i18nc("@label:group prefix for radio button group", "Sidebar position:")
            text: mirrored ? i18nc("@option:radio sidebar position", "Right") : i18nc("option:radio sidebar position", "Left")
            QQC2.ButtonGroup.group: paneSwapGroup
            property int index: 0
            checked: !Plasmoid.configuration.paneSwap
        }

        QQC2.RadioButton {
            id: paneSwapOn
            text: mirrored ? i18nc("@option:radio sidebar position", "Left") : i18nc("@option:radio sidebar position", "Right")
            QQC2.ButtonGroup.group: paneSwapGroup
            property int index: 1
            checked: Plasmoid.configuration.paneSwap
        }

        QQC2.RadioButton {
            id: showFavoritesInGrid
            Kirigami.FormData.label: i18nc("@title:group prefix for radio button group", "Show favorites:")
            text: i18nc("@option:radio Part of a sentence: 'Show favorites in a grid'", "In a grid")
            QQC2.ButtonGroup.group: favoritesDisplayGroup
            property int index: 0
            checked: Plasmoid.configuration.favoritesDisplay === index
        }

        QQC2.RadioButton {
            id: showFavoritesInList
            text: i18nc("@option:radio Part of a sentence: 'Show favorites in a list'", "In a list")
            QQC2.ButtonGroup.group: favoritesDisplayGroup
            property int index: 1
            checked: Plasmoid.configuration.favoritesDisplay === index
        }

        QQC2.RadioButton {
            id: showAppsInGrid
            Kirigami.FormData.label: i18nc("@title:group prefix for radio button group", "Show other applications:")
            text: i18nc("@option:radio Part of a sentence: 'Show other applications in a grid'", "In a grid")
            QQC2.ButtonGroup.group: applicationsDisplayGroup
            property int index: 0
            checked: Plasmoid.configuration.applicationsDisplay === index
        }

        QQC2.RadioButton {
            id: showAppsInList
            text: i18nc("@option:radio Part of a sentence: 'Show other applications in a list'", "In a list")
            QQC2.ButtonGroup.group: applicationsDisplayGroup
            property int index: 1
            checked: Plasmoid.configuration.applicationsDisplay === index
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.RadioButton {
            id: powerActionsButton
            Kirigami.FormData.label: i18nc("@title:group prefix for radio button group", "Show buttons for:")
            text: i18nc("@option:radio Show buttons for", "Power")
            QQC2.ButtonGroup.group: radioGroup
            property string actions: "suspend,hibernate,reboot,shutdown"
            property int index: 0
            checked: Plasmoid.configuration.primaryActions === index
        }

        QQC2.RadioButton {
            id: sessionActionsButton
            text: i18nc("@option:radio Show buttons for", "Session")
            QQC2.ButtonGroup.group: radioGroup
            property string actions: "lock-screen,logout,save-session,switch-user"
            property int index: 1
            checked: Plasmoid.configuration.primaryActions === index
        }

        QQC2.RadioButton {
            id: allActionsButton
            text: i18nc("@option:radio Show buttons for", "Power and session")
            QQC2.ButtonGroup.group: radioGroup
            property string actions: "lock-screen,logout,save-session,switch-user,suspend,hibernate,reboot,shutdown"
            property int index: 3
            checked: Plasmoid.configuration.primaryActions === index
        }

        QQC2.CheckBox {
            id: showActionButtonCaptions
            text: i18nc("@option:check", "Show action button captions")
        }
    }

    QQC2.ButtonGroup {
        id: paneSwapGroup
        onCheckedButtonChanged: {
            if (checkedButton) {
                root.cfg_paneSwap = checkedButton.index === 1
            }
        }
    }

    QQC2.ButtonGroup {
        id: favoritesDisplayGroup
        onCheckedButtonChanged: {
            if (checkedButton) {
                root.cfg_favoritesDisplay = checkedButton.index
            }
        }
    }

    QQC2.ButtonGroup {
        id: applicationsDisplayGroup
        onCheckedButtonChanged: {
            if (checkedButton) {
                root.cfg_applicationsDisplay = checkedButton.index
            }
        }
    }

    QQC2.ButtonGroup {
        id: radioGroup
        onCheckedButtonChanged: {
            if (checkedButton) {
                root.cfg_primaryActions = checkedButton.index
                root.cfg_systemFavorites = checkedButton.actions
            }
        }
    }
}
