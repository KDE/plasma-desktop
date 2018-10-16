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
import org.kde.kirigami 2.4 as Kirigami

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
        showDesktopEntry: false

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

    Kirigami.FormLayout {
        anchors.horizontalCenter: parent.horizontalCenter

        RadioButton {
            id: locationDesktop

            Kirigami.FormData.label: i18n("Show:")

            text: i18n("Desktop folder")
            exclusiveGroup: locationGroup
        }

        RadioButton {
            id: locationCurrentActivity
            visible: placesModel.activityLinkingEnabled

            text: i18n("Files linked to the current activity")
            exclusiveGroup: locationGroup
        }

        RadioButton {
            id: locationPlace

            text: i18n("Places panel item:")

            exclusiveGroup: locationGroup

            onCheckedChanged: {
                locationPlaceValue.enabled = checked;
            }
        }
        RowLayout {
            Layout.fillWidth: true

            Item {
                width: units.largeSpacing
            }
            ComboBox {
                id: locationPlaceValue

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
        }

        RadioButton {
            id: locationCustom

            exclusiveGroup: locationGroup
            text: i18n("Custom location:")
        }

        RowLayout {
            Layout.fillWidth: true
            Item {
                width: units.largeSpacing
            }
            TextField {
                id: locationCustomValue
                enabled: locationCustom.checked
                Layout.fillWidth: true

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
        }

        Item {
            visible: titleVisible
            Kirigami.FormData.isSection: true
        }

        ComboBox {
            id: labelMode
            visible: titleVisible
            Layout.fillWidth: true

            Kirigami.FormData.label: i18n("Title:")

            model: [i18n("None"), i18n("Default"), i18n("Full path"), i18n("Custom title")]
        }

        RowLayout {
            Layout.fillWidth: true
            visible: titleVisible

            Item {
                width: units.largeSpacing
            }
            TextField {
                id: labelText
                Layout.fillWidth: true
                enabled: (labelMode.currentIndex == 3)

                placeholderText: i18n("Enter custom title here")
            }
        }
    }
}
