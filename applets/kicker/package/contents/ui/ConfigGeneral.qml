/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.draganddrop as DragDrop
import org.kde.iconthemes as KIconThemes
import org.kde.kcmutils as KCMUtils
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid

KCMUtils.SimpleKCM {
    id: configGeneral

    property bool isDash: (Plasmoid.pluginName === "org.kde.plasma.kickerdash")

    property string cfg_icon: Plasmoid.configuration.icon
    property bool cfg_useCustomButtonImage: Plasmoid.configuration.useCustomButtonImage
    property string cfg_customButtonImage: Plasmoid.configuration.customButtonImage

    property alias cfg_appNameFormat: appNameFormat.currentIndex
    property alias cfg_limitDepth: limitDepth.checked
    property alias cfg_alphaSort: alphaSort.checked
    property alias cfg_showIconsRootLevel: showIconsRootLevel.checked

    property alias cfg_recentOrdering: recentOrdering.currentIndex
    property alias cfg_showRecentApps: showRecentApps.checked
    property alias cfg_showRecentDocs: showRecentDocs.checked

    property alias cfg_useExtraRunners: useExtraRunners.checked
    property alias cfg_alignResultsToBottom: alignResultsToBottom.checked

    Kirigami.FormLayout {
        anchors.left: parent.left
        anchors.right: parent.right

        QQC2.Button {
            id: iconButton

            Kirigami.FormData.label: i18n("Icon:")

            implicitWidth: previewFrame.width + Kirigami.Units.smallSpacing * 2
            implicitHeight: previewFrame.height + Kirigami.Units.smallSpacing * 2

            // Just to provide some visual feedback when dragging;
            // cannot have checked without checkable enabled
            checkable: true
            checked: dropArea.containsAcceptableDrag

            onPressed: iconMenu.opened ? iconMenu.close() : iconMenu.open()

            DragDrop.DropArea {
                id: dropArea

                property bool containsAcceptableDrag: false

                anchors.fill: parent

                onDragEnter: event => {
                    // Cannot use string operations (e.g. indexOf()) on "url" basic type.
                    const urlString = event.mimeData.url.toString();

                    // This list is also hardcoded in KIconDialog.
                    const extensions = [".png", ".xpm", ".svg", ".svgz"];
                    containsAcceptableDrag = urlString.startsWith("file:///")
                        && extensions.some(extension => urlString.endsWith(extension));

                    if (!containsAcceptableDrag) {
                        event.ignore();
                    }
                }
                onDragLeave: event => {
                    containsAcceptableDrag = false
                }

                onDrop: event => {
                    if (containsAcceptableDrag) {
                        // Strip file:// prefix, we already verified in onDragEnter that we have only local URLs.
                        iconDialog.setCustomButtonImage(event.mimeData.url.toString().substr("file://".length));
                    }
                    containsAcceptableDrag = false;
                }
            }

            KIconThemes.IconDialog {
                id: iconDialog

                function setCustomButtonImage(image) {
                    configGeneral.cfg_customButtonImage = image || configGeneral.cfg_icon || "start-here-kde-symbolic"
                    configGeneral.cfg_useCustomButtonImage = true;
                }

                onIconNameChanged: iconName => {
                    setCustomButtonImage(iconName);
                }
            }

            KSvg.FrameSvgItem {
                id: previewFrame
                anchors.centerIn: parent
                imagePath: Plasmoid.location === PlasmaCore.Types.Vertical || Plasmoid.location === PlasmaCore.Types.Horizontal
                        ? "widgets/panel-background" : "widgets/background"
                width: Kirigami.Units.iconSizes.large + fixedMargins.left + fixedMargins.right
                height: Kirigami.Units.iconSizes.large + fixedMargins.top + fixedMargins.bottom

                Kirigami.Icon {
                    anchors.centerIn: parent
                    width: Kirigami.Units.iconSizes.large
                    height: width
                    source: configGeneral.cfg_useCustomButtonImage ? configGeneral.cfg_customButtonImage : configGeneral.cfg_icon
                }
            }

            QQC2.Menu {
                id: iconMenu

                // Appear below the button
                y: parent.height

                onClosed: iconButton.checked = false;

                QQC2.MenuItem {
                    text: i18nc("@item:inmenu Open icon chooser dialog", "Choose…")
                    icon.name: "document-open-folder"
                    onClicked: iconDialog.open()
                }
                QQC2.MenuItem {
                    text: i18nc("@item:inmenu Reset icon to default", "Clear Icon")
                    icon.name: "edit-clear"
                    onClicked: {
                        configGeneral.cfg_icon = "start-here-kde-symbolic"
                        configGeneral.cfg_useCustomButtonImage = false
                    }
                }
            }
        }


        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.ComboBox {
            id: appNameFormat

            Kirigami.FormData.label: i18n("Show applications as:")

            model: [i18n("Name only"), i18n("Description only"), i18n("Name (Description)"), i18n("Description (Name)")]
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.CheckBox {
            id: alphaSort

            Kirigami.FormData.label: i18n("Behavior:")

            text: i18n("Sort applications alphabetically")
        }

        QQC2.CheckBox {
            id: limitDepth

            visible: !isDash

            text: i18n("Flatten sub-menus to a single level")
        }

        QQC2.CheckBox {
            id: showIconsRootLevel

            visible: !configGeneral.isDash

            text: i18n("Show icons on the root level of the menu")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.CheckBox {
            id: showRecentApps

            Kirigami.FormData.label: i18n("Show categories:")

            text: recentOrdering.currentIndex == 0
                    ? i18n("Recent applications")
                    : i18n("Often used applications")
        }

        QQC2.CheckBox {
            id: showRecentDocs

            text: recentOrdering.currentIndex == 0
                    ? i18n("Recent files")
                    : i18n("Often used files")
        }

        QQC2.ComboBox {
            id: recentOrdering

            Kirigami.FormData.label: i18n("Sort items in categories by:")
            model: [i18nc("@item:inlistbox Sort items in categories by [Recently used | Often used]", "Recently used"), i18nc("@item:inlistbox Sort items in categories by [Recently used | Ofetn used]", "Often used")]
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.CheckBox {
            id: useExtraRunners

            Kirigami.FormData.label: i18n("Search:")

            text: i18n("Expand search to bookmarks, files and emails")
        }

        QQC2.CheckBox {
            id: alignResultsToBottom

            visible: !configGeneral.isDash

            text: i18n("Align search results to bottom")
        }
    }
}
