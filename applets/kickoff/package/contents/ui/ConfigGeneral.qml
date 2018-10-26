/*
 *  Copyright 2013 David Edmundson <davidedmundson@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.0 as QtControls

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.kquickcontrolsaddons 2.0 as KQuickAddons

import org.kde.plasma.extras 2.0 as PlasmaExtras

ColumnLayout {
    property string cfg_icon: plasmoid.configuration.icon
    property alias cfg_switchTabsOnHover: switchTabsOnHoverCheckbox.checked
    property alias cfg_showAppsByName: showApplicationsByNameCheckbox.checked
    property alias cfg_useExtraRunners: useExtraRunners.checked
    property alias cfg_alphaSort: alphaSort.checked
    property alias cfg_menuItems: configButtons.menuItems

    spacing: units.smallSpacing

    RowLayout {
        spacing: units.smallSpacing

        QtControls.Label {
            text: i18n("Icon:")
        }

        QtControls.Button {
            id: iconButton
            Layout.minimumWidth: previewFrame.width + units.smallSpacing * 2
            Layout.maximumWidth: Layout.minimumWidth
            Layout.minimumHeight: previewFrame.height + units.smallSpacing * 2
            Layout.maximumHeight: Layout.minimumWidth

            KQuickAddons.IconDialog {
                id: iconDialog
                onIconNameChanged: cfg_icon = iconName || "start-here-kde" // TODO use actual default
            }

            // just to provide some visual feedback, cannot have checked without checkable enabled
            checkable: true
            onClicked: {
                checked = Qt.binding(function() { // never actually allow it being checked
                    return iconMenu.status === PlasmaComponents.DialogStatus.Open
                })

                iconMenu.open(0, height)
            }

            PlasmaCore.FrameSvgItem {
                id: previewFrame
                anchors.centerIn: parent
                imagePath: plasmoid.location === PlasmaCore.Types.Vertical || plasmoid.location === PlasmaCore.Types.Horizontal
                         ? "widgets/panel-background" : "widgets/background"
                width: units.iconSizes.large + fixedMargins.left + fixedMargins.right
                height: units.iconSizes.large + fixedMargins.top + fixedMargins.bottom

                PlasmaCore.IconItem {
                    anchors.centerIn: parent
                    width: units.iconSizes.large
                    height: width
                    source: cfg_icon
                }
            }
        }

        // QQC Menu can only be opened at cursor position, not a random one
        PlasmaComponents.ContextMenu {
            id: iconMenu
            visualParent: iconButton

            PlasmaComponents.MenuItem {
                text: i18nc("@item:inmenu Open icon chooser dialog", "Choose...")
                icon: "document-open-folder"
                onClicked: iconDialog.open()
            }
            PlasmaComponents.MenuItem {
                text: i18nc("@item:inmenu Reset icon to default", "Clear Icon")
                icon: "edit-clear"
                onClicked: cfg_icon = "start-here-kde" // TODO reset to actual default
            }
        }
    }

    QtControls.CheckBox {
        id: switchTabsOnHoverCheckbox
        text: i18n("Switch tabs on hover")
    }

    QtControls.CheckBox {
        id: showApplicationsByNameCheckbox
        text: i18n("Show applications by name")
    }

    QtControls.CheckBox {
        id: useExtraRunners
        text: i18n("Expand search to bookmarks, files and emails")
    }

    QtControls.CheckBox {
        id: alphaSort
        text: i18n("Sort alphabetically")
    }

    Item {
        width: height
        height: units.gridUnit / 2
    }

    SystemPalette {
        id: palette
    }

    PlasmaExtras.Heading {
        level: 2
        text: i18n("Menu Buttons")
        color: palette.text
    }

    Row {
        spacing: units.gridUnit
        Column {
            QtControls.Label {
                text: i18n("Visible Tabs")
                height: configButtons.cellHeight
            }
            QtControls.Label {
                text: i18n("Hidden Tabs")
                height: configButtons.cellHeight
            }
        }
        Column {
            ConfigButtons {
                id: configButtons
            }
        }
    }

    QtControls.Label {
        Layout.fillWidth: true
        text: i18n("Drag tabs between the boxes to show/hide them, or reorder the visible tabs by dragging.")
        wrapMode: Text.WordWrap
    }

    Item {
        //spacer
        Layout.fillHeight: true
    }
}
