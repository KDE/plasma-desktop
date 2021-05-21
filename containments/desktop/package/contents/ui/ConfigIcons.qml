/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kconfig 1.0 // for KAuthorized
import org.kde.kirigami 2.5 as Kirigami

import org.kde.private.desktopcontainment.desktop 0.1 as Desktop
import org.kde.private.desktopcontainment.folder 0.1 as Folder

Item {
    id: configIcons

    property bool isPopup: (plasmoid.location !== PlasmaCore.Types.Floating)

    property string cfg_icon: plasmoid.configuration.icon
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
    property alias cfg_previewPlugins: previewPluginsDialog.previewPlugins
    property alias cfg_viewMode: viewMode.currentIndex
    property alias cfg_iconSize: iconSize.value
    property alias cfg_labelWidth: labelWidth.currentIndex
    property alias cfg_textLines: textLines.value

    readonly property bool lockedByKiosk: !KAuthorized.authorize("editable_desktop_icons")

    IconDialog {
        id: iconDialog
        onIconNameChanged: cfg_icon = iconName || "folder"
    }

    Kirigami.FormLayout {
        anchors.horizontalCenter: parent.horizontalCenter


        // Panel button
        RowLayout {
            spacing: PlasmaCore.Units.smallSpacing
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
                Layout.minimumWidth: PlasmaCore.Units.iconSizes.large + PlasmaCore.Units.smallSpacing * 2
                Layout.maximumWidth: Layout.minimumWidth
                Layout.minimumHeight: Layout.minimumWidth
                Layout.maximumHeight: Layout.minimumWidth

                checkable: true
                enabled: useCustomIcon.checked

                onClicked: {
                    checked = Qt.binding(function() {
                        return iconMenu.status === PlasmaComponents.DialogStatus.Open;
                    })

                    iconMenu.open(0, height);
                }

                PlasmaCore.IconItem {
                    anchors.centerIn: parent
                    width: PlasmaCore.Units.iconSizes.large
                    height: width
                    source: cfg_icon
                }
            }

            PlasmaComponents.ContextMenu {
                id: iconMenu
                visualParent: iconButton

                PlasmaComponents.MenuItem {
                    text: i18nc("@item:inmenu Open icon chooser dialog", "Choose…")
                    icon: "document-open-folder"
                    onClicked: iconDialog.open()
                }

                PlasmaComponents.MenuItem {
                    text: i18nc("@item:inmenu Reset icon to default", "Clear Icon")
                    icon: "edit-clear"
                    onClicked: cfg_icon = "folder"
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

            Kirigami.FormData.label: i18n("Arrangement:")

            model: [i18n("Rows"), i18n("Columns")]
        }

        ComboBox {
            id: alignment
            Layout.fillWidth: true

            model: [i18n("Align left"), i18n("Align right")]
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
        }


        // Sorting section
        ComboBox {
            id: sortMode
            Layout.fillWidth: true

            Kirigami.FormData.label: i18n("Sorting:")

            property int mode
            // FIXME TODO HACK: This maps the combo box list model to the KDirModel::ModelColumns
            // enum, which should be done in C++.
            property variant indexToMode: [-1, 0, 1, 6, 2]
            property variant modeToIndex: {'-1' : '0', '0' : '1', '1' : '2', '6' : '3', '2' : '4'}

            model: [i18n("Manual"), i18n("Name"), i18n("Size"), i18n("Type"), i18n("Date")]

            Component.onCompleted: currentIndex = modeToIndex[mode]
            onActivated: mode = indexToMode[index]
        }

        CheckBox {
            id: sortDesc

            enabled: (sortMode.currentIndex != 0)

            text: i18n("Descending")
        }

        CheckBox {
            id: sortDirsFirst

            enabled: (sortMode.currentIndex != 0)

            text: i18n("Folders first")
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

            model: [i18n("List"), i18n("Icons")]
        }


        // Size section
        Slider {
            id: iconSize

            Layout.fillWidth: true
            visible: !isPopup || viewMode.currentIndex === 1

            Kirigami.FormData.label: i18n("Icon size:")

            from: 0
            to: 5
            stepSize: 1
            snapMode: Slider.SnapAlways
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                Layout.alignment: Qt.AlignLeft
                visible: !isPopup || viewMode.currentIndex === 1

                text: i18n("Small")
            }
            Item {
                Layout.fillWidth: true
            }
            Label {
                Layout.alignment: Qt.AlignRight
                visible: !isPopup || viewMode.currentIndex === 1

                text: i18n("Large")
            }
        }

        ComboBox {
            id: labelWidth
            visible: !isPopup || viewMode.currentIndex === 1
            Layout.fillWidth: true

            Kirigami.FormData.label: i18n("Label width:")

            model: [
                i18n("Narrow"),
                i18n("Medium"),
                i18n("Wide")
            ]
        }

        SpinBox {
            id: textLines
            visible: !isPopup || viewMode.currentIndex === 1

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

            Kirigami.FormData.label: i18n("Features:")

            text: i18n("Tooltips")
        }

        CheckBox {
            id: selectionMarkers
            visible: Qt.styleHints.singleClickActivation

            text: i18n("Selection markers")
        }

        CheckBox {
            id: renameInline
            visible: !selectionMarkers.visible

            text: i18n("Rename inline by clicking selected item's text")
        }

        CheckBox {
            id: popups
            visible: !isPopup

            text: i18n("Folder preview popups")
        }

        CheckBox {
            id: previews

            text: i18n("Preview thumbnails")
        }

        Button {
            id: previewSettings
            Layout.fillWidth: true

            icon.name: "configure"
            text: i18n("Configure Preview Plugins…")

            onClicked: {
                previewPluginsDialog.visible = true;
            }
        }
    }

    FolderItemPreviewPluginsDialog {
        id: previewPluginsDialog
    }
}
