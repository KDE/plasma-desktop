/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kcmutils as KCM

import org.kde.private.desktopcontainment.folder as Folder

KCM.SimpleKCM {
    id: configLocation

    property string cfg_url
    property alias cfg_labelMode: labelMode.currentIndex
    property alias cfg_labelText: labelText.text
    property bool titleVisible: Plasmoid.containment != Plasmoid

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
                locationPlaceValue.currentIndex = placeForUrl; // needs to happen before checking the radiobutton
                locationPlace.checked = true;
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

        RadioButton {
            id: locationDesktop
            implicitHeight: locationCustomValue.implicitHeight

            Kirigami.FormData.label: i18nc("@title:group form label for radiobutton group", "Show:")

            text: i18nc("@option:radio", "Desktop folder")
        }

        RadioButton {
            id: locationCurrentActivity
            visible: placesModel.activityLinkingEnabled
            implicitHeight: locationCustomValue.implicitHeight

            text: i18nc("@option:radio", "Files linked to the current activity")
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            RadioButton {
                id: locationPlace

                text: i18nc("@option:radio also label for combobox", "Places panel item:")
                Layout.minimumWidth: Math.max(locationPlace.implicitWidth, locationCustom.implicitWidth, labelMode.implicitWidth)

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
                        cfg_url = Folder.DesktopSchemeHelper.getDesktopUrl(placesModel.urlForIndex(currentIndex));
                    }
                }

                onActivated: index => {
                    cfg_url = Folder.DesktopSchemeHelper.getDesktopUrl(placesModel.urlForIndex(index));
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            RadioButton {
                id: locationCustom
                Layout.minimumWidth: Math.max(locationPlace.implicitWidth, locationCustom.implicitWidth, labelMode.implicitWidth)

                text: i18nc("@option:radio also label for text field", "Custom location:")
            }

            TextField {
                id: locationCustomValue
                enabled: locationCustom.checked
                Layout.fillWidth: true

                placeholderText: i18nc("@info:placeholder custom location", "Type path or URL…")

                inputMethodHints: Qt.ImhNoPredictiveText

                onEnabledChanged: {
                    if (enabled && text !== "") {
                        cfg_url = Folder.DesktopSchemeHelper.getDesktopUrl(text);
                    }
                }

                onTextChanged: {
                    if (enabled) {
                        cfg_url = Folder.DesktopSchemeHelper.getDesktopUrl(text);
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
                    locationCustomValue.text = Folder.DesktopSchemeHelper.getDesktopUrl(url);
                }
            }
        }

        Item {
            visible: titleVisible
            Kirigami.FormData.isSection: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            visible: titleVisible
            Kirigami.FormData.label: i18nc("@label:textbox custom widget title", "Title:")

            ComboBox {
                id: labelMode
                Layout.minimumWidth: Math.max(locationPlace.implicitWidth, locationCustom.implicitWidth, labelMode.implicitWidth)
                visible: titleVisible


                model: [
                    i18nc("@item:inlistbox no title", "None"),
                    i18nc("@item:inlistbox default title", "Default"),
                    i18nc("@item:inlistbox full path as title", "Full path"),
                    i18nc("@item:inlistbox title from text input field", "Custom")
                ]
            }

            TextField {
                id: labelText
                Layout.fillWidth: true
                enabled: (labelMode.currentIndex === 3)

                placeholderText: i18nc("@info:placeholder custom window title", "Enter title…")
            }
        }
    }
}
