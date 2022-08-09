/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami

import org.kde.private.desktopcontainment.folder 0.1 as Folder

Item {
    id: configLocation

    property string cfg_url
    property alias cfg_labelMode: labelMode.currentIndex
    property alias cfg_labelText: labelText.text
    property bool titleVisible: !("containmentType" in plasmoid)

    onCfg_urlChanged: applyConfig()

    function applyConfig(force) {
        if (!force && locationGroup.checkedButton !== null) {
            return;
        }

        if (cfg_url === "desktop:/") {
            locationDesktop.checked = true;
            locationCustomValue.text = "";
        } else if (cfg_url === "activities:/current/") {
            locationCurrentActivity.checked = true;
            locationCustomValue.text = "";
        } else {
            var placeForUrl = placesModel.indexForUrl(cfg_url);

            if (placeForUrl !== -1) {
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

    ButtonGroup {
        id: locationGroup

        buttons: [locationDesktop, locationCurrentActivity, locationPlace, locationCustom]

        onCheckedButtonChanged: {
            if (checkedButton === locationDesktop) {
                cfg_url = "desktop:/";
            } else if (checkedButton === locationCurrentActivity) {
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
        }

        RadioButton {
            id: locationCurrentActivity
            visible: placesModel.activityLinkingEnabled

            text: i18n("Files linked to the current activity")
        }

        RowLayout {
            RadioButton {
                id: locationPlace

                text: i18n("Places panel item:")

                onCheckedChanged: {
                    locationPlaceValue.enabled = checked;
                }
            }

            ComboBox {
                id: locationPlaceValue

                Layout.fillWidth: true

                model: placesModel
                textRole: "display"

                enabled: true

                onEnabledChanged: {
                    if (enabled && currentIndex !== -1) {
                        cfg_url = placesModel.urlForIndex(currentIndex);
                    }
                }

                onActivated: {
                    cfg_url = placesModel.urlForIndex(index);
                }
            }
        }

        RowLayout {
            RadioButton {
                id: locationCustom

                text: i18n("Custom location:")
            }

            TextField {
                id: locationCustomValue
                enabled: locationCustom.checked
                Layout.fillWidth: true

                placeholderText: i18n("Type path or URL…")

                inputMethodHints: Qt.ImhNoPredictiveText

                onEnabledChanged: {
                    if (enabled && text !== "") {
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
                icon.name: "document-open"

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

            Kirigami.FormData.label: i18n("Title:")

            model: [i18n("None"), i18n("Default"), i18n("Full path"), i18n("Custom title")]
        }

        RowLayout {
            visible: titleVisible

            Item {
                width: PlasmaCore.Units.largeSpacing
            }

            TextField {
                id: labelText
                Layout.fillWidth: true
                enabled: (labelMode.currentIndex === 3)

                placeholderText: i18n("Enter custom title…")
            }
        }
    }
}
