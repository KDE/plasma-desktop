/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kcmutils as KCMUtils
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid
import org.kde.plasma.private.kicker as Kicker

KCMUtils.SimpleKCM {
    id: configGeneral

    readonly property bool isDash: (Plasmoid.pluginName === "org.kde.plasma.kickerdash")

    property string cfg_icon: Plasmoid.configuration.icon
    property alias cfg_menuLabel: menuLabel.text

    property alias cfg_appNameFormat: appNameFormat.currentIndex
    property alias cfg_limitDepth: limitDepth.checked
    property alias cfg_alphaSort: alphaSort.checked
    property alias cfg_showIconsRootLevel: showIconsRootLevel.checked

    property alias cfg_recentOrdering: recentOrdering.currentIndex
    property alias cfg_showRecentApps: showRecentApps.checked
    property alias cfg_showRecentDocs: showRecentDocs.checked
    property alias cfg_showRecentFolders: showRecentFolders.checked

    property alias cfg_highlightNewlyInstalledApps: highlightNewlyInstalledAppsCheckbox.checked
    property alias cfg_useExtraRunners: useExtraRunners.checked
    property alias cfg_alignResultsToBottom: alignResultsToBottom.checked
    property alias cfg_forceDarkMode: forceDarkMode.checked

    Kirigami.FormLayout {
        anchors.left: parent.left
        anchors.right: parent.right

        Kicker.IconButton {
            id: iconButton
            Kirigami.FormData.label: i18nc("@label prefix for icon-only button", "Icon:")
            hasLabelText: menuLabel.text != ""
            widgetName: configGeneral.isDash
                ? i18nc("@label match translation with Kicker's metadata.json", "Application Menu")
                : i18nc("@label match translation with Kickerdash's metadata.json in kdeplasma-addons", "Application Dashboard")
        }

        Kirigami.ActionTextField {
            id: menuLabel
            Kirigami.FormData.label: i18nc("@label:textbox", "Text label:")
            placeholderText: i18nc("@info:placeholder", "Type here to add a text label")
            onTextEdited: {
                // This is to make sure that we always have a icon if there is no text.
                // If the user remove the icon and remove the text, without this, we'll have no icon and no text.
                // This is to force the icon to be there.

                if (!menuLabel.text) {
                    configGeneral.cfg_icon = configGeneral.cfg_icon || Kicker.Tools.defaultIconName
                }
            }
            rightActions: QQC2.Action {
                icon.name: "edit-clear"
                enabled: menuLabel.text !== ""
                text: i18nc("@action:button", "Reset menu label")
                onTriggered: {
                    menuLabel.clear()
                    configGeneral.cfg_icon = configGeneral.cfg_icon || Kicker.Tools.defaultIconName
                }
            }
        }


        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.ComboBox {
            id: appNameFormat

            visible: !configGeneral.isDash

            Kirigami.FormData.label: i18nc("@label:listbox", "Show applications as:")

            model: [i18nc("@item:inlistbox", "Name only"), i18nc("@item:inlistbox", "Description only"),
                    i18nc("@item:inlistbox", "Name (Description)"), i18nc("@item:inlistbox", "Description (Name)")]
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.CheckBox {
            id: alphaSort

            Kirigami.FormData.label: i18nc("@title:group prefix for checkbox group", "Behavior:")

            text: i18nc("@option:check", "Sort applications alphabetically")
        }

        QQC2.CheckBox {
            id: limitDepth

            visible: !configGeneral.isDash

            text: i18nc("@option:check", "Flatten sub-menus to a single level")
        }

        QQC2.CheckBox {
            id: showIconsRootLevel

            visible: !configGeneral.isDash

            text: i18nc("@option:check", "Show icons on the root level of the menu")
        }

        QQC2.CheckBox {
            id: highlightNewlyInstalledAppsCheckbox

            text: i18nc("@option:check", "Highlight newly-installed applications")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.CheckBox {
            id: showRecentApps

            Kirigami.FormData.label: i18nc("@title:group prefix for checkbox group", "Show categories:")

            text: recentOrdering.currentIndex == 0
                    ? i18nc("@option:check", "Recent applications")
                    : i18nc("@option:check", "Often used applications")
        }

        QQC2.CheckBox {
            id: showRecentDocs

            text: recentOrdering.currentIndex == 0
                    ? i18nc("@option:check", "Recent files")
                    : i18nc("@option:check", "Often used files")
        }

        QQC2.CheckBox {
            id: showRecentFolders

            text: recentOrdering.currentIndex == 0
                    ? i18nc("@option:check", "Recent places")
                    : i18nc("@option:check", "Often used places")
        }

        QQC2.ComboBox {
            id: recentOrdering

            Kirigami.FormData.label: i18nc("@label:listbox", "Sort items in categories by:")
            model: [i18nc("@item:inlistbox Sort items in categories by [Recently used | Often used]", "Recently used"), i18nc("@item:inlistbox Sort items in categories by [Recently used | Often used]", "Often used")]
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.CheckBox {
            id: useExtraRunners

            Kirigami.FormData.label: i18nc("@title:group prefix for checkbox group", "Search:")

            text: i18nc("@option:check", "Expand search to bookmarks, files and emails")
        }

        QQC2.CheckBox {
            id: alignResultsToBottom

            visible: !configGeneral.isDash

            text: i18nc("@option:check", "Align search results to bottom")
        }

        RowLayout {
            visible: configGeneral.isDash
            Kirigami.FormData.label: i18nc("@label prefix for a checkbox toggling Dark Mode", "Appearance:")
            spacing: Kirigami.Units.smallSpacing

            QQC2.CheckBox {
                id: forceDarkMode
                text: i18nc("@option:check", "Prefer Dark Mode when available")
            }

            Kirigami.ContextualHelpButton {
                toolTipText: xi18nc("@info:tooltip", "This feature uses the complementary colors from your colorscheme, which usually feature a dark background.")
            }
        }
    }
}
