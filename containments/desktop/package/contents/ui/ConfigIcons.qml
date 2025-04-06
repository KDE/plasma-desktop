/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.iconthemes as KIconThemes
import org.kde.config // for KAuthorized
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kcmutils as KCM

import org.kde.private.desktopcontainment.folder as Folder

KCM.SimpleKCM {
    id: configIcons

    property bool isPopup: (Plasmoid.location !== PlasmaCore.Types.Floating)

    property string cfg_icon: Plasmoid.configuration.icon
    property alias cfg_useCustomIcon: useCustomIcon.checked
    property alias cfg_arrangement: arrangement.currentIndex
    property alias cfg_alignment: alignment.currentIndex
    property bool cfg_locked
    property alias cfg_sortMode: sortMode.mode
    property alias cfg_sortDesc: sortDesc.checked
    property alias cfg_sortDirsFirst: sortDirsFirst.checked
    property alias cfg_toolTips: toolTips.checked
    property alias cfg_selectionMarkers: selectionMarkers.checked
    property alias cfg_renameInline: renameInline.checked
    property alias cfg_popups: popups.checked
    property alias cfg_previews: previews.checked
    property var cfg_previewPlugins
    property alias cfg_viewMode: viewMode.currentIndex
    property alias cfg_iconSize: iconSize.value
    property alias cfg_labelWidth: labelWidth.currentIndex
    property alias cfg_textLines: textLines.value

    readonly property bool lockedByKiosk: !KAuthorized.authorize("editable_desktop_icons")

    KIconThemes.IconDialog {
        id: iconDialog
        onIconNameChanged: iconName => cfg_icon = iconName || "folder-symbolic";
    }

    Kirigami.FormLayout {
        // Panel button
        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            visible: isPopup

            Kirigami.FormData.label: i18n("Panel button:")

            CheckBox {
                id: useCustomIcon
                visible: isPopup
                checked: cfg_useCustomIcon
                text: i18n("Use a custom icon")
            }

            Button {
                id: iconButton
                Layout.minimumWidth: Kirigami.Units.iconSizes.large + Kirigami.Units.smallSpacing * 2
                Layout.maximumWidth: Layout.minimumWidth
                Layout.minimumHeight: Layout.minimumWidth
                Layout.maximumHeight: Layout.minimumWidth

                checkable: true
                enabled: useCustomIcon.checked

                onClicked: {
                    checked = Qt.binding(() =>
                        iconMenu.status === PlasmaExtras.Menu.Open);

                    iconMenu.open(0, height);
                }

                Kirigami.Icon {
                    anchors.centerIn: parent
                    width: Kirigami.Units.iconSizes.large
                    height: width
                    source: cfg_icon
                }
            }

            PlasmaExtras.Menu {
                id: iconMenu
                visualParent: iconButton

                PlasmaExtras.MenuItem {
                    text: i18nc("@item:inmenu Open icon chooser dialog", "Choose…")
                    icon: "document-open-folder"
                    onClicked: iconDialog.open()
                }

                PlasmaExtras.MenuItem {
                    text: i18nc("@item:inmenu Reset icon to default", "Clear Icon")
                    icon: "edit-clear"
                    onClicked: cfg_icon = "folder-symbolic";
                }
            }
        }

        Item {
            visible: isPopup
            Kirigami.FormData.isSection: true
        }



        // Arrangement section
        ComboBox {
            id: arrangement
            Layout.fillWidth: true
            visible: !isPopup || viewMode.currentIndex === 1 /* Icons mode */

            Kirigami.FormData.label: i18n("Arrangement:")

            model: [
                Qt.application.layoutDirection === Qt.LeftToRight ?
                    i18nc("@item:inlistbox arrangement of icons", "Left to Right") :
                    i18nc("@item:inlistbox arrangement of icons", "Right to Left"),
                i18nc("@item:inlistbox arrangement of icons", "Top to Bottom"),
            ]
        }

        ComboBox {
            id: alignment
            Layout.fillWidth: true
            visible: !isPopup || viewMode.currentIndex === 1 /* Icons mode */

            model: [i18nc("@item:inlistbox alignment of icons", "Align left"),
                    i18nc("@item:inlistbox alignment of icons", "Align right")]
        }

        CheckBox {
            id: locked
            visible: ("containmentType" in plasmoid)
            checked: cfg_locked || lockedByKiosk
            enabled: !lockedByKiosk

            onCheckedChanged: {
                if (!lockedByKiosk) {
                    cfg_locked = checked;
                }
            }

            text: i18n("Lock in place")
        }

        Item {
            Kirigami.FormData.isSection: true
            visible: !isPopup || viewMode.currentIndex === 1 /* Icons mode */
        }


        // Sorting section
        ComboBox {
            id: sortMode
            Layout.fillWidth: true

            Kirigami.FormData.label: i18n("Sorting:")

            property int mode
            // FIXME TODO HACK: This maps the combo box list model to the KDirModel::ModelColumns
            // enum, which should be done in C++.
            property var indexToMode: [-1, 0, 1, 6, 2]
            property var modeToIndex: {'-1': '0', '0': '1', '1': '2', '6': '3', '2': '4'}

            model: [i18nc("@item:inlistbox sort icons manually", "Manual"),
                    i18nc("@item:inlistbox sort icons by name", "Name"),
                    i18nc("@item:inlistbox sort icons by size", "Size"),
                    i18nc("@item:inlistbox sort icons by file type", "Type"),
                    i18nc("@item:inlistbox sort icons by date", "Date")]

            Component.onCompleted: currentIndex = modeToIndex[mode]
            onActivated: mode = indexToMode[index]
        }

        CheckBox {
            id: sortDesc

            enabled: sortMode.currentIndex !== 0

            text: i18nc("@option:check sort icons in descending order", "Descending")
        }

        CheckBox {
            id: sortDirsFirst

            enabled: sortMode.currentIndex !== 0

            text: i18nc("@option:check sort icons with folders first", "Folders first")
        }

        Item {
            Kirigami.FormData.isSection: true
        }


        // View Mode section (only if we're a pop-up)
        ComboBox {
            id: viewMode
            visible: isPopup
            Layout.fillWidth: true

            Kirigami.FormData.label: i18nc("whether to use icon or list view", "View mode:")

            model: [i18nc("@item:inlistbox show icons in a list", "List"),
                    i18nc("@item:inlistbox show icons in a grid", "Grid")]
        }


        // Size section
        Slider {
            id: iconSize

            Layout.fillWidth: true
            visible: !isPopup || viewMode.currentIndex === 1 /* Icons mode */

            Kirigami.FormData.label: i18n("Icon size:")

            from: 0
            to: 6
            stepSize: 1
            snapMode: Slider.SnapAlways
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                Layout.alignment: Qt.AlignLeft
                visible: !isPopup || viewMode.currentIndex === 1 /* Icons mode */

                text: i18nc("@label:slider smallest icon size", "Small")
            }
            Item {
                Layout.fillWidth: true
            }
            Label {
                Layout.alignment: Qt.AlignRight
                visible: !isPopup || viewMode.currentIndex === 1 /* Icons mode */

                text: i18nc("@label:slider largest icon size", "Large")
            }
        }

        ComboBox {
            id: labelWidth
            visible: !isPopup || viewMode.currentIndex === 1 /* Icons mode */
            Layout.fillWidth: true

            Kirigami.FormData.label: i18n("Label width:")

            model: [
                i18nc("@item:inlistbox how long a text label should be", "Narrow"),
                i18nc("@item:inlistbox how long a text label should be", "Medium"),
                i18nc("@item:inlistbox how long a text label should be", "Wide"),
            ]
        }

        SpinBox {
            id: textLines
            visible: !isPopup || viewMode.currentIndex === 1 /* Icons mode */

            Kirigami.FormData.label: i18n("Text lines:")

            from: 1
            to: 10
            stepSize: 1
        }

        Item {
            Kirigami.FormData.isSection: true
        }


        // Features section
        CheckBox {
            id: toolTips

            Kirigami.FormData.label: i18n("When hovering over icons:")

            text: i18n("Show tooltips")
        }

        CheckBox {
            id: selectionMarkers
            visible: Qt.styleHints.singleClickActivation

            text: i18n("Show selection markers")
        }

        CheckBox {
            id: popups
            visible: !isPopup

            text: i18n("Show folder preview popups")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        CheckBox {
            id: renameInline

            Kirigami.FormData.label: i18n("Rename:")

            visible: !selectionMarkers.visible

            text: i18n("Rename inline by clicking selected item's text")
        }

        Item {
            Kirigami.FormData.isSection: true
            visible: renameInline.visible
        }

        CheckBox {
            id: previews

            Kirigami.FormData.label: i18n("Previews:")

            text: i18n("Show preview thumbnails")
        }

        Button {
            id: previewSettings
            Layout.fillWidth: true

            icon.name: "configure"
            text: i18n("Configure Preview Plugins…")

            onClicked: {
                const component = Qt.createComponent(Qt.resolvedUrl("FolderItemPreviewPluginsDialog.qml"));
                component.incubateObject(configIcons.Window.window.contentItem, {
                    "previewPlugins": configIcons.cfg_previewPlugins,
                }, Qt.Asynchronous);
                component.destroy();
            }
        }
    }
}
