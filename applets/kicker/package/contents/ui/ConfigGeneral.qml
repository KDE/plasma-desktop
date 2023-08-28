/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls

import org.kde.kirigami 2 as Kirigami
import org.kde.iconthemes as KIconThemes
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.plasmoid 2.0

Kirigami.FormLayout {
    id: configGeneral

    anchors.left: parent.left
    anchors.right: parent.right

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

    Button {
        id: iconButton

        Kirigami.FormData.label: i18n("Icon:")

        implicitWidth: previewFrame.width + Kirigami.Units.smallSpacing * 2
        implicitHeight: previewFrame.height + Kirigami.Units.smallSpacing * 2

        // Just to provide some visual feedback when dragging;
        // cannot have checked without checkable enabled
        checkable: true
        checked: dropArea.containsAcceptableDrag

        onPressed: iconMenu.opened ? iconMenu.close() : iconMenu.open()

        DropArea {
            id: dropArea

            anchors.fill: parent

            property bool containsAcceptableDrag: false

            onEntered: event =>{
                if (!event.hasUrls) {
                    event.accepted = false;
                    return;
                }
                // Cannot use string operations (e.g. indexOf()) on "url" basic type.
                const urlString = event.urls[0].toString();

                // This list is also hardcoded in KIconDialog.
                const extensions = [".png", ".xpm", ".svg", ".svgz"];
                containsAcceptableDrag = urlString.indexOf("file:///") === 0 && extensions.some(function (extension) {
                    return urlString.indexOf(extension) === urlString.length - extension.length; // "endsWith"
                });

                if (!containsAcceptableDrag) {
                    event.accepted = false;
                }
            }
            onExited: containsAcceptableDrag = false

            onDropped: event => {
                if (containsAcceptableDrag) {
                    // Strip file:// prefix, we already verified in onDragEnter that we have only local URLs.
                    iconDialog.setCustomButtonImage(event.urls[0].toString().substr("file://".length));
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

            onIconNameChanged: setCustomButtonImage(iconName);
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

        Menu {
            id: iconMenu

            // Appear below the button
            y: +parent.height

            onClosed: iconButton.checked = false;

            MenuItem {
                text: i18nc("@item:inmenu Open icon chooser dialog", "Choose…")
                icon.name: "document-open-folder"
                onClicked: iconDialog.open()
            }
            MenuItem {
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

    ComboBox {
        id: appNameFormat

        Kirigami.FormData.label: i18n("Show applications as:")

        model: [i18n("Name only"), i18n("Description only"), i18n("Name (Description)"), i18n("Description (Name)")]
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    CheckBox {
        id: alphaSort

        Kirigami.FormData.label: i18n("Behavior:")

        text: i18n("Sort applications alphabetically")
    }

    CheckBox {
        id: limitDepth

        visible: !isDash

        text: i18n("Flatten sub-menus to a single level")
    }

    CheckBox {
        id: showIconsRootLevel

        visible: !configGeneral.isDash

        text: i18n("Show icons on the root level of the menu")
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    CheckBox {
        id: showRecentApps

        Kirigami.FormData.label: i18n("Show categories:")

        text: recentOrdering.currentIndex == 0
                ? i18n("Recent applications")
                : i18n("Often used applications")
    }

    CheckBox {
        id: showRecentDocs

        text: recentOrdering.currentIndex == 0
                ? i18n("Recent files")
                : i18n("Often used files")
    }

    ComboBox {
        id: recentOrdering

        Kirigami.FormData.label: i18n("Sort items in categories by:")
        model: [i18nc("@item:inlistbox Sort items in categories by [Recently used | Often used]", "Recently used"), i18nc("@item:inlistbox Sort items in categories by [Recently used | Ofetn used]", "Often used")]
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    CheckBox {
        id: useExtraRunners

        Kirigami.FormData.label: i18n("Search:")

        text: i18n("Expand search to bookmarks, files and emails")
    }

    CheckBox {
        id: alignResultsToBottom

        visible: !configGeneral.isDash

        text: i18n("Align search results to bottom")
    }
}
