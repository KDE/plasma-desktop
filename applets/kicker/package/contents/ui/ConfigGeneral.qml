/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.kde.draganddrop 2.0 as DragDrop
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kquickcontrolsaddons 2.0 as KQuickAddons
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.plasma.private.kicker 0.1 as Kicker

Kirigami.FormLayout {
    id: configGeneral

    anchors.left: parent.left
    anchors.right: parent.right

    property bool isDash: (plasmoid.pluginName === "org.kde.plasma.kickerdash")

    property string cfg_icon: plasmoid.configuration.icon
    property bool cfg_useCustomButtonImage: plasmoid.configuration.useCustomButtonImage
    property string cfg_customButtonImage: plasmoid.configuration.customButtonImage

    property alias cfg_appNameFormat: appNameFormat.currentIndex
    property alias cfg_limitDepth: limitDepth.checked
    property alias cfg_alphaSort: alphaSort.checked
    property alias cfg_showIconsRootLevel: showIconsRootLevel.checked

    property alias cfg_recentOrdering: recentOrdering.currentIndex
    property alias cfg_showRecentApps: showRecentApps.checked
    property alias cfg_showRecentDocs: showRecentDocs.checked
    property alias cfg_showRecentContacts: showRecentContacts.checked

    property alias cfg_useExtraRunners: useExtraRunners.checked
    property alias cfg_alignResultsToBottom: alignResultsToBottom.checked


    Button {
        id: iconButton

        Kirigami.FormData.label: i18n("Icon:")

        implicitWidth: previewFrame.width + PlasmaCore.Units.smallSpacing * 2
        implicitHeight: previewFrame.height + PlasmaCore.Units.smallSpacing * 2

        // Just to provide some visual feedback when dragging;
        // cannot have checked without checkable enabled
        checkable: true
        checked: dropArea.containsAcceptableDrag

        onPressed: iconMenu.opened ? iconMenu.close() : iconMenu.open()

        DragDrop.DropArea {
            id: dropArea

            property bool containsAcceptableDrag: false

            anchors.fill: parent

            onDragEnter: {
                // Cannot use string operations (e.g. indexOf()) on "url" basic type.
                var urlString = event.mimeData.url.toString();

                // This list is also hardcoded in KIconDialog.
                var extensions = [".png", ".xpm", ".svg", ".svgz"];
                containsAcceptableDrag = urlString.indexOf("file:///") === 0 && extensions.some(function (extension) {
                    return urlString.indexOf(extension) === urlString.length - extension.length; // "endsWith"
                });

                if (!containsAcceptableDrag) {
                    event.ignore();
                }
            }
            onDragLeave: containsAcceptableDrag = false

            onDrop: {
                if (containsAcceptableDrag) {
                    // Strip file:// prefix, we already verified in onDragEnter that we have only local URLs.
                    iconDialog.setCustomButtonImage(event.mimeData.url.toString().substr("file://".length));
                }
                containsAcceptableDrag = false;
            }
        }

        KQuickAddons.IconDialog {
            id: iconDialog

            function setCustomButtonImage(image) {
                configGeneral.cfg_customButtonImage = image || configGeneral.cfg_icon || "start-here-kde"
                configGeneral.cfg_useCustomButtonImage = true;
            }

            onIconNameChanged: setCustomButtonImage(iconName);
        }

        PlasmaCore.FrameSvgItem {
            id: previewFrame
            anchors.centerIn: parent
            imagePath: plasmoid.location === PlasmaCore.Types.Vertical || plasmoid.location === PlasmaCore.Types.Horizontal
                    ? "widgets/panel-background" : "widgets/background"
            width: PlasmaCore.Units.iconSizes.large + fixedMargins.left + fixedMargins.right
            height: PlasmaCore.Units.iconSizes.large + fixedMargins.top + fixedMargins.bottom

            PlasmaCore.IconItem {
                anchors.centerIn: parent
                width: PlasmaCore.Units.iconSizes.large
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
                    configGeneral.cfg_icon = "start-here-kde"
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

    CheckBox {
        id: showRecentContacts

        text: recentOrdering.currentIndex == 0
                ? i18n("Recent contacts")
                : i18n("Often used contacts")
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
