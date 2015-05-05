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

    ColumnLayout {
        GroupBox {
            id: locationGroupBox

            Layout.fillWidth: true

            title: i18n("Location")
            flat: true

            ColumnLayout {
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

                RadioButton {
                    id: locationDesktop
                    text: i18n("Show the Desktop folder")
                    exclusiveGroup: locationGroup
                }

                RadioButton {
                    id: locationCurrentActivity
                    visible: placesModel.activityLinkingEnabled
                    text: i18n("Show files linked to the current activity")
                    exclusiveGroup: locationGroup
                }

                GridLayout {
                    RadioButton {
                        id: locationPlace

                        Layout.column: 0
                        Layout.rowSpan: 2
                        Layout.alignment: Qt.AlignTop

                        exclusiveGroup: locationGroup

                        onCheckedChanged: {
                            locationPlaceValue.enabled = checked;
                        }
                    }

                    Label {
                        Layout.row: 0
                        Layout.column: 1
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true

                        text: i18n("Show a place:")
                    }

                    ComboBox {
                        id: locationPlaceValue

                        Layout.row: 1
                        Layout.column: 1
                        Layout.fillWidth: false

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

                GridLayout {
                    RadioButton {
                        id: locationCustom

                        Layout.column: 0
                        Layout.rowSpan: 2
                        Layout.alignment: Qt.AlignTop

                        exclusiveGroup: locationGroup
                    }

                    Label {
                        Layout.row: 0
                        Layout.column: 1
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true

                        text: i18n("Specify a folder:")
                    }

                    TextField {
                        id: locationCustomValue

                        Layout.row: 1
                        Layout.column: 1
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
            }
        }

        GroupBox {
            id: titleGroupBox

            Layout.fillWidth: true

            visible: !("containmentType" in plasmoid)

            title: i18n("Title")

            flat: true

            ColumnLayout {
                ComboBox {
                    id: labelMode

                    Layout.fillWidth: false

                    model: [i18n("None"), i18n("Default"), i18n("Full path"), i18n("Custom title")]
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
}
