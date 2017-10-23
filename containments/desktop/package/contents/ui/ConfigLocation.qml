/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

import org.kde.plasma.plasmoid 2.0

import org.kde.private.desktopcontainment.folder 0.1 as Folder

Item {
    id: configLocation

    width: childrenRect.width
    height: childrenRect.height

    property string cfg_url
    property alias cfg_labelMode: labelMode.currentIndex
    property alias cfg_labelText: labelText.text
    property bool titleVisible: !("containmentType" in plasmoid)

    onCfg_urlChanged: applyConfig()

    function applyConfig(force) {
        if (!force && locationGroup.current != null) {
            return;
        }

        if (cfg_url == "desktop:/") {
            locationDesktop.checked = true;
            locationCustomValue.text = "";
        } else if (cfg_url == "activities:/current/") {
            locationCurrentActivity.checked = true;
            locationCustomValue.text = "";
        } else {
            var placeForUrl = placesModel.indexForUrl(cfg_url);

            if (placeForUrl != -1) {
                locationPlace.checked = true;
                locationPlaceValue.currentIndex = placeForUrl;
                locationCustomValue.text = "";
            } else {
                locationCustom.checked = true;
                locationCustomValue.text = cfg_url;
            }
        }

        locationPlaceValue.enabled = locationPlace.checked;
    }

    Folder.PlacesModel {
        id: placesModel

        onPlacesChanged: applyConfig(true)
    }

    ExclusiveGroup {
        id: locationGroup

        onCurrentChanged: {
            if (current == locationDesktop) {
                cfg_url = "desktop:/";
            } else if (current == locationCurrentActivity) {
                cfg_url = "activities:/current/";
            }
        }
    }

    GridLayout {

        // Row 0: "Show the Desktop folder"
        Label {
            id: locationLabel
            Layout.column: 0
            Layout.row: 0
            text: i18n("Location:")
        }

        RadioButton {
            id: locationDesktop
            Layout.row: 0
            Layout.column: 1
            Layout.columnSpan: 3
            text: i18n("Show the Desktop folder")
            exclusiveGroup: locationGroup
        }

        // Row 1: "Show files linked to the current activity"
        RadioButton {
            id: locationCurrentActivity
            Layout.row: 1
            Layout.column: 1
            Layout.columnSpan: 3

            visible: placesModel.activityLinkingEnabled
            text: i18n("Show files linked to the current activity")
            exclusiveGroup: locationGroup
        }

        // Rows 2+3: "Show a place"
        RadioButton {
            id: locationPlace
            Layout.row: 2
            Layout.column: 1
            Layout.columnSpan: 3
            text: i18n("Show a place:")

            exclusiveGroup: locationGroup

            onCheckedChanged: {
                locationPlaceValue.enabled = checked;
            }
        }

        Item {
            id: indentSpacer
            Layout.row: 3
            Layout.column: 1
            Layout.minimumWidth: units.largeSpacing
        }

        ComboBox {
            id: locationPlaceValue
            Layout.row: 3
            Layout.column: 2
            Layout.columnSpan: 2
            Layout.fillWidth: true

            model: placesModel
            textRole: "display"

            enabled: true

            onEnabledChanged: {
                if (enabled && currentIndex != -1) {
                    cfg_url = placesModel.urlForIndex(currentIndex);
                }
            }

            onActivated: {
                cfg_url = placesModel.urlForIndex(index);
            }
        }

        // Rows 4+5: "Specify a folder"

        RadioButton {
            id: locationCustom
            Layout.row: 4
            Layout.column: 1
            Layout.columnSpan: 3

            exclusiveGroup: locationGroup
            text: i18n("Specify a folder:")
        }

        TextField {
            id: locationCustomValue
            Layout.row: 5
            Layout.column: 2
            Layout.fillWidth: true

            enabled: locationCustom.checked

            placeholderText: i18n("Type a path or a URL here")

            onEnabledChanged: {
                if (enabled && text != "") {
                    cfg_url = text;
                }
            }

            onTextChanged: {
                if (enabled) {
                    cfg_url = text;
                }
            }
        }

        Button {
            Layout.row: 5
            Layout.column: 3
            Layout.alignment: Qt.AlignLeft
            iconName: "document-open"

            enabled: locationCustom.checked

            onClicked: {
                directoryPicker.open();
            }
        }

        Folder.DirectoryPicker {
            id: directoryPicker

            onUrlChanged: {
                locationCustomValue.text = url;
            }
        }

        // Row 6: Spacing
        Item {
            id: titleSpacer
            Layout.column: 0
            Layout.row: 6
            Layout.minimumHeight: units.largeSpacing
            visible: titleVisible
        }

        // Rows 7+8: "Title"
        Label {
            id: titleLabel
            Layout.column: 0
            Layout.row: 7
            text: i18n("Title:")
            visible: titleVisible
        }

        ComboBox {
            id: labelMode
            Layout.row: 7
            Layout.column: 1
            Layout.columnSpan: 3
            Layout.fillWidth: true
            visible: titleVisible

            model: [i18n("None"), i18n("Default"), i18n("Full path"), i18n("Custom title")]
        }

        TextField {
            id: labelText
            Layout.row: 8
            Layout.column: 2
            Layout.columnSpan: 2
            Layout.fillWidth: true
            visible: titleVisible

            enabled: (labelMode.currentIndex == 3)

            placeholderText: i18n("Enter custom title here")
        }
    }
}
