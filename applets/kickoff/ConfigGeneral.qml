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
import org.kde.ksvg as KSvg
import org.kde.iconthemes as KIconThemes
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM
import org.kde.config as KConfig
import org.kde.plasma.plasmoid

import "code/tools.js" as Tools

KCM.SimpleKCM {
    id: root

    property string cfg_menuLabel: menuLabel.text
    property string cfg_icon: Plasmoid.configuration.icon
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
        QQC2.Button {
            id: iconButton

            Kirigami.FormData.label: i18n("Icon:") // qmllint disable unqualified

            implicitWidth: previewFrame.width + Kirigami.Units.smallSpacing * 2
            implicitHeight: previewFrame.height + Kirigami.Units.smallSpacing * 2
            hoverEnabled: true

            Accessible.name: i18nc("@action:button", "Change Application Launcher's icon") // qmllint disable unqualified
            Accessible.description: i18nc("@info:whatsthis", "Current icon is %1. Click to open menu to change the current icon or reset to the default icon.", root.cfg_icon) // qmllint disable unqualified
            Accessible.role: Accessible.ButtonMenu

            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.text: i18nc("@info:tooltip", "Icon name is \"%1\"", root.cfg_icon) // qmllint disable unqualified
            QQC2.ToolTip.visible: iconButton.hovered && root.cfg_icon.length > 0

            KIconThemes.IconDialog {
                id: iconDialog
                onIconNameChanged: {
                    root.cfg_icon = iconName || Tools.defaultIconName;
                }
            }

            onPressed: iconMenu.opened ? iconMenu.close() : iconMenu.open()

            KSvg.FrameSvgItem {
                id: previewFrame
                anchors.centerIn: parent
                imagePath: Plasmoid.formFactor === PlasmaCore.Types.Vertical || Plasmoid.formFactor === PlasmaCore.Types.Horizontal
                        ? "widgets/panel-background" : "widgets/background"
                width: Kirigami.Units.iconSizes.large + fixedMargins.left + fixedMargins.right
                height: Kirigami.Units.iconSizes.large + fixedMargins.top + fixedMargins.bottom

                Kirigami.Icon {
                    anchors.centerIn: parent
                    width: Kirigami.Units.iconSizes.large
                    height: width
                    source: Tools.iconOrDefault(Plasmoid.formFactor, root.cfg_icon)
                }
            }

            QQC2.Menu {
                id: iconMenu

                // Appear below the button
                y: parent.height

                QQC2.MenuItem {
                    text: i18nc("@item:inmenu Open icon chooser dialog", "Choose…") // qmllint disable unqualified
                    icon.name: "document-open-folder"
                    Accessible.description: i18nc("@info:whatsthis", "Choose an icon for Application Launcher") // qmllint disable unqualified
                    onClicked: iconDialog.open()
                }
                QQC2.MenuItem {
                    text: i18nc("@item:inmenu Reset icon to default", "Reset to default icon") // qmllint disable unqualified
                    icon.name: "edit-clear"
                    enabled: root.cfg_icon !== Tools.defaultIconName
                    onClicked: root.cfg_icon = Tools.defaultIconName
                }
                QQC2.MenuItem {
                    text: i18nc("@action:inmenu", "Remove icon") // qmllint disable unqualified
                    icon.name: "delete"
                    enabled: root.cfg_icon !== "" && menuLabel.text && Plasmoid.formFactor !== PlasmaCore.Types.Vertical
                    onClicked: root.cfg_icon = ""
                }
            }
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.ComboBox {
            id: appNameFormat

            Kirigami.FormData.label: i18n("Show applications as:") // qmllint disable unqualified

            model: [i18n("Name only"), i18n("Description only"), i18n("Name (Description)"), i18n("Description (Name)")] // qmllint disable unqualified
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        Kirigami.ActionTextField {
            id: menuLabel
            enabled: Plasmoid.formFactor !== PlasmaCore.Types.Vertical
            Kirigami.FormData.label: i18nc("@label:textbox", "Text label:") // qmllint disable unqualified
            text: Plasmoid.configuration.menuLabel
            placeholderText: i18nc("@info:placeholder", "Type here to add a text label") // qmllint disable unqualified
            onTextEdited: {
                root.cfg_menuLabel = menuLabel.text

                // This is to make sure that we always have a icon if there is no text.
                // If the user remove the icon and remove the text, without this, we'll have no icon and no text.
                // This is to force the icon to be there.
                if (!menuLabel.text) {
                    root.cfg_icon = root.cfg_icon || Tools.defaultIconName
                }
            }
            rightActions: QQC2.Action {
                icon.name: "edit-clear"
                enabled: menuLabel.text !== ""
                text: i18nc("@action:button", "Reset menu label") // qmllint disable unqualified
                onTriggered: {
                    menuLabel.clear()
                    root.cfg_menuLabel = ""
                    root.cfg_icon = root.cfg_icon || Tools.defaultIconName
                }
            }
        }

        QQC2.Label {
            Layout.fillWidth: true
            Layout.maximumWidth: Kirigami.Units.gridUnit * 25
            visible: Plasmoid.formFactor === PlasmaCore.Types.Vertical
            text: i18nc("@info", "A text label cannot be set when the Panel is vertical.") // qmllint disable unqualified
            wrapMode: Text.Wrap
            font: Kirigami.Theme.smallFont
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        RowLayout {
            Kirigami.FormData.label: i18nc("General options", "General:") // qmllint disable unqualified
            spacing: Kirigami.Units.smallSpacing
            QQC2.CheckBox {
                id: alphaSort
                text: i18nc("@option:check", "Sort applications alphabetically") // qmllint disable unqualified
            }

            Kirigami.ContextualHelpButton {
                toolTipText: i18nc("@info:whatsthis", "This doesn't affect how applications are sorted in either search results or the favorites page.") // qmllint disable unqualified
            }
        }

        QQC2.CheckBox {
            id: compactModeCheckbox
            text: i18n("Use compact list item style") // qmllint disable unqualified
            checked: Kirigami.Settings.tabletMode ? true : Plasmoid.configuration.compactMode
            enabled: !Kirigami.Settings.tabletMode
        }
        QQC2.Label {
            visible: Kirigami.Settings.tabletMode
            text: i18nc("@info:usagetip under a checkbox when Touch Mode is on", "Automatically disabled when in Touch Mode") // qmllint disable unqualified
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            font: Kirigami.Theme.smallFont
        }

        QQC2.CheckBox {
            id: highlightNewlyInstalledAppsCheckbox
            text: i18n("Highlight newly installed applications") // qmllint disable unqualified
        }

        QQC2.CheckBox {
            id: switchCategoryOnHoverCheckbox
            text: i18n("Switch sidebar categories when hovering over them") // qmllint disable unqualified
        }

        QQC2.Button {
            enabled: KConfig.KAuthorized.authorizeControlModule("kcm_plasmasearch")
            icon.name: "settings-configure"
            text: i18nc("@action:button opens plasmasearch kcm", "Configure Search Plugins…") // qmllint disable unqualified
            onClicked: KCM.KCMLauncher.openSystemSettings("kcm_plasmasearch")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.RadioButton {
            id: paneSwapOff
            Kirigami.FormData.label: i18n("Sidebar position:") // qmllint disable unqualified
            text: mirrored ? i18n("Right") : i18n("Left") // qmllint disable unqualified
            QQC2.ButtonGroup.group: paneSwapGroup
            property int index: 0
            checked: !Plasmoid.configuration.paneSwap
        }

        QQC2.RadioButton {
            id: paneSwapOn
            text: mirrored ? i18n("Left") : i18n("Right") // qmllint disable unqualified
            QQC2.ButtonGroup.group: paneSwapGroup
            property int index: 1
            checked: Plasmoid.configuration.paneSwap
        }

        QQC2.RadioButton {
            id: showFavoritesInGrid
            Kirigami.FormData.label: i18n("Show favorites:") // qmllint disable unqualified
            text: i18nc("Part of a sentence: 'Show favorites in a grid'", "In a grid") // qmllint disable unqualified
            QQC2.ButtonGroup.group: favoritesDisplayGroup
            property int index: 0
            checked: Plasmoid.configuration.favoritesDisplay === index
        }

        QQC2.RadioButton {
            id: showFavoritesInList
            text: i18nc("Part of a sentence: 'Show favorites in a list'", "In a list") // qmllint disable unqualified
            QQC2.ButtonGroup.group: favoritesDisplayGroup
            property int index: 1
            checked: Plasmoid.configuration.favoritesDisplay === index
        }

        QQC2.RadioButton {
            id: showAppsInGrid
            Kirigami.FormData.label: i18n("Show other applications:") // qmllint disable unqualified
            text: i18nc("Part of a sentence: 'Show other applications in a grid'", "In a grid") // qmllint disable unqualified
            QQC2.ButtonGroup.group: applicationsDisplayGroup
            property int index: 0
            checked: Plasmoid.configuration.applicationsDisplay === index
        }

        QQC2.RadioButton {
            id: showAppsInList
            text: i18nc("Part of a sentence: 'Show other applications in a list'", "In a list") // qmllint disable unqualified
            QQC2.ButtonGroup.group: applicationsDisplayGroup
            property int index: 1
            checked: Plasmoid.configuration.applicationsDisplay === index
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.RadioButton {
            id: powerActionsButton
            Kirigami.FormData.label: i18n("Show buttons for:") // qmllint disable unqualified
            text: i18n("Power") // qmllint disable unqualified
            QQC2.ButtonGroup.group: radioGroup
            property string actions: "suspend,hibernate,reboot,shutdown"
            property int index: 0
            checked: Plasmoid.configuration.primaryActions === index
        }

        QQC2.RadioButton {
            id: sessionActionsButton
            text: i18n("Session") // qmllint disable unqualified
            QQC2.ButtonGroup.group: radioGroup
            property string actions: "lock-screen,logout,save-session,switch-user"
            property int index: 1
            checked: Plasmoid.configuration.primaryActions === index
        }

        QQC2.RadioButton {
            id: allActionsButton
            text: i18n("Power and session") // qmllint disable unqualified
            QQC2.ButtonGroup.group: radioGroup
            property string actions: "lock-screen,logout,save-session,switch-user,suspend,hibernate,reboot,shutdown"
            property int index: 3
            checked: Plasmoid.configuration.primaryActions === index
        }

        QQC2.CheckBox {
            id: showActionButtonCaptions
            text: i18n("Show action button captions") // qmllint disable unqualified
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
