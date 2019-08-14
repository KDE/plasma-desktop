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

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.5

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kquickcontrolsaddons 2.0 as KQuickAddons
import org.kde.kirigami 2.5 as Kirigami

ColumnLayout {

    property string cfg_icon: plasmoid.configuration.icon
    property alias cfg_switchTabsOnHover: switchTabsOnHoverCheckbox.checked
    property alias cfg_showAppsByName: showApplicationsByNameCheckbox.checked
    property alias cfg_useExtraRunners: useExtraRunners.checked
    property alias cfg_alphaSort: alphaSort.checked
    property alias cfg_menuItems: configButtons.menuItems

    Kirigami.FormLayout {
        Button {
            id: iconButton

            Kirigami.FormData.label: i18n("Icon:")

            implicitWidth: previewFrame.width + units.smallSpacing * 2
            implicitHeight: previewFrame.height + units.smallSpacing * 2

            KQuickAddons.IconDialog {
                id: iconDialog
                onIconNameChanged: cfg_icon = iconName || "start-here-kde"
            }

            onPressed: iconMenu.opened ? iconMenu.close() : iconMenu.open()

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

            Menu {
                id: iconMenu

                // Appear below the button
                y: +parent.height

                MenuItem {
                    text: i18nc("@item:inmenu Open icon chooser dialog", "Choose...")
                    icon.name: "document-open-folder"
                    onClicked: iconDialog.open()
                }
                MenuItem {
                    text: i18nc("@item:inmenu Reset icon to default", "Clear Icon")
                    icon.name: "edit-clear"
                    onClicked: cfg_icon = "start-here-kde"
                }
            }
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        CheckBox {
            id: switchTabsOnHoverCheckbox

            Kirigami.FormData.label: i18n("General:")

            text: i18n("Switch tabs on hover")
        }

        CheckBox {
            id: showApplicationsByNameCheckbox
            text: i18n("Show applications by name")
        }

        CheckBox {
            id: useExtraRunners
            text: i18n("Expand search to bookmarks, files and emails")
        }

        CheckBox {
            id: alphaSort
            text: i18n("Sort alphabetically")
        }
    }

    ConfigButtons {
        id: configButtons
        Layout.alignment: Qt.AlignHCenter
    }
    Label {
        Layout.fillWidth: true
        text: i18n("Drag tabs between the boxes to show/hide them, or reorder the visible tabs by dragging.")
        wrapMode: Text.WordWrap
    }

    Item {
        Layout.fillHeight: true
    }
}
