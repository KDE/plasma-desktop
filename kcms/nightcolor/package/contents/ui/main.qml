/********************************************************************
Copyright 2017 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.5 as QQC2

import org.kde.kirigami 2.5 as Kirigami
import org.kde.kcm 1.2 as KCM

import org.kde.colorcorrect 0.1 as CC

KCM.SimpleKCM {
    id: root
    property int error: cA.error
    property bool defaultRequested: false
    implicitHeight: Kirigami.Units.gridUnit * 29
    implicitWidth: Kirigami.Units.gridUnit * 35

    CC.CompositorAdaptor {
        id: cA
    }

    CC.Geolocator {
        id: locator
    }

    CC.SunCalc {
        id: sunCalc
    }

    function calcNeedsSave() {
        kcm.needsSave = cA.checkStaged();
        // If the user changes something manually again after
        // Default was triggered, only allow a normal
        // configuration change again:
        defaultRequested = false;

        calcRepresentsDefault();
    }
    function calcRepresentsDefault() {
        if (cA.nightColorAvailable) {
            kcm.representsDefaults = activator.checked === cA.activeDefault &&
                tempSlider.value === cA.nightTemperatureDefault &&
                modeSwitcher.currentIndex === cA.modeDefault &&
                cA.latitudeFixedStaged === cA.latitudeFixedDefault &&
                cA.longitudeFixedStaged === cA.longitudeFixedDefault &&
                cA.morningBeginFixedStaged.toString() === cA.morningBeginFixedDefault.toString() &&
                cA.eveningBeginFixedStaged.toString() === cA.eveningBeginFixedDefault.toString() &&
                cA.transitionTimeStaged === cA.transitionTimeDefault;
        } else {
            kcm.representsDefaults = true;
        }
    }

    Connections {
        target: kcm
        onLoadRelay: cA.reloadData()
        onSaveRelay: {
            defaultRequested ? cA.sendConfigurationAll() : cA.sendConfiguration();
        }
        onDefaultsRelay: {
            if (!cA.nightColorAvailable) {
                return;
            }
            activator.checked = cA.activeDefault;
            tempSlider.value = cA.nightTemperatureDefault;
            modeSwitcher.currentIndex = cA.modeDefault;

            // set everything else to default as well
            defaults()

            mornBeginManField.backend = cA.morningBeginFixedDefault;
            evenBeginManField.backend = cA.eveningBeginFixedDefault;
            transTimeField.value = cA.transitionTimeDefault;
            cA.transitionTimeStaged = transTimeField.value;

            kcm.needsSave = cA.checkStagedAll();
            defaultRequested = true;

            calcRepresentsDefault();
        }
    }
    signal defaults()

    Connections {
        target: cA
        onDataUpdated: calcNeedsSave()
        onStagedDataReset: {
            activator.checked = cA.active;
            tempSlider.value = cA.nightTemperature;
            modeSwitcher.currentIndex = cA.mode;
            reset();
            calcNeedsSave();
        }
    }
    signal reset()

    header: ColumnLayout{
        Kirigami.InlineMessage {
            id: errorMessage
            Layout.fillWidth: true
            visible: error != CC.CompositorAdaptor.ErrorCodeSuccess
            type: Kirigami.MessageType.Error
            text: cA.errorText
        }
    }

    ColumnLayout {
        spacing: 0

        QQC2.Label {
            Layout.topMargin: Kirigami.Units.largeSpacing * 2
            Layout.bottomMargin: Kirigami.Units.largeSpacing * 4
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: Math.round(root.width * 0.5)

            text: i18n("Night Color makes the colors on the screen warmer to reduce eye strain at the time of your choosing.")
            wrapMode: Text.WordWrap
        }

        Kirigami.FormLayout {
            id: parentLayout

            Connections {
                target: root
                onReset: {
                    mornBeginManField.backend = cA.morningBeginFixed;
                    evenBeginManField.backend = cA.eveningBeginFixed;
                    transTimeField.value = cA.transitionTime;
                }
            }

            QQC2.CheckBox {
                id: activator
                text: i18n("Activate Night Color")
                enabled: cA.nightColorAvailable
                checked: cA.active

                onToggled: {
                    cA.activeStaged = checked;
                    calcNeedsSave();
                }
            }

            Item {
                Kirigami.FormData.isSection: true
            }

            GridLayout {
                Kirigami.FormData.label: i18n("Night Color Temperature:")
                Kirigami.FormData.buddyFor: tempSlider
                enabled: activator.checked

                columns: 4

                QQC2.Slider {
                    id: tempSlider
                    // Match combobox width
                    Layout.minimumWidth: modeSwitcher.width
                    enabled: activator.checked
                    from: cA.minimalTemperature
                    to: cA.neutralTemperature
                    value: cA.nightTemperature
                    stepSize: 100

                    Layout.columnSpan: 3

                    onValueChanged: {
                        cA.nightTemperatureStaged = value;
                        calcNeedsSave();
                    }
                }
                QQC2.Label {
                        text: tempSlider.value + i18n(" K")
                }
                //row 2
                QQC2.Label {
                    text: i18nc("Night colour red-ish", "Warm")
                }
                Item {
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: i18nc("Night colour blue-ish", "Cool")
                }
                Item {}
            }

            Item {
                Kirigami.FormData.isSection: true
            }

            QQC2.ComboBox {
                id: modeSwitcher
                // Work around https://bugs.kde.org/show_bug.cgi?id=403153
                Layout.minimumWidth: Kirigami.Units.gridUnit * 17
                Kirigami.FormData.label: i18n("Activation time:")
                enabled: activator.checked
                model: [
                    i18n("Sunset to sunrise at current location"),
                    i18n("Sunset to sunrise at manual location"),
                    i18n("Custom time"),
                    i18n("Always on")
                ]
                currentIndex: cA.mode
                onCurrentIndexChanged: {
                    cA.modeStaged = currentIndex;
                    calcNeedsSave();
                }
            }

            // Show current location in auto mode
            QQC2.Label {
                visible: modeSwitcher.currentIndex === CC.CompositorAdaptor.ModeAutomatic
                enabled: activator.checked
                wrapMode: Text.Wrap
                text: i18n("Latitude: %1   Longitude: %2", Math.round(locator.latitude * 100)/100, Math.round(locator.longitude * 100)/100)
            }

            // Show time entry fields in manual timings mode
            TimeField {
                id: evenBeginManField
                // Match combobox width
                Layout.minimumWidth: modeSwitcher.width
                visible: modeSwitcher.currentIndex === CC.CompositorAdaptor.ModeTimings
                Kirigami.FormData.label: i18n("Turn on at:")
                backend: cA.eveningBeginFixedStaged
                onBackendChanged: {
                    cA.eveningBeginFixedStaged = backend;
                    calcNeedsSave();
                }

                QQC2.ToolTip {
                    text: i18n("Input format: HH:MM")
                }
            }

            TimeField {
                id: mornBeginManField
                // Match combobox width
                Layout.minimumWidth: modeSwitcher.width
                visible: modeSwitcher.currentIndex === CC.CompositorAdaptor.ModeTimings
                Kirigami.FormData.label: i18n("Turn off at:")
                backend: cA.morningBeginFixedStaged
                onBackendChanged: {
                    cA.morningBeginFixedStaged = backend;
                    calcNeedsSave();
                }

                QQC2.ToolTip {
                    text: i18n("Input format: HH:MM")
                }
            }

            QQC2.SpinBox {
                id: transTimeField
                visible: modeSwitcher.currentIndex === CC.CompositorAdaptor.ModeTimings
                // Match width of combobox and input fields
                Layout.minimumWidth: modeSwitcher.width
                Kirigami.FormData.label: i18n("Transition duration:")
                from: 1
                to: 600 // less than 12 hours (in minutes: 720)
                value: cA.transitionTimeStaged
                editable: true
                onValueModified: {
                    cA.transitionTimeStaged = value;
                    calcNeedsSave();
                }
                textFromValue: function(value, locale) {
                    return i18np("%1 minute", "%1 minutes", value)
                }
                valueFromText: function(text, locale) {
                    return parseInt(text);
                }

                QQC2.ToolTip {
                    text: i18n("Input minutes - min. 1, max. 600")
                }
            }

            QQC2.Label {
                id: manualTimingsError1
                visible: evenBeginManField.getNormedDate() - mornBeginManField.getNormedDate() <= 0
                font.italic: true
                text: i18n("Error: Morning is before evening.")
            }

            QQC2.Label {
                id: manualTimingsError2
                visible: {
                    if (manualTimingsError1.visible) {
                        return false;
                    }
                    var trTime = transTimeField.backend * 60 * 1000;
                    var mor = mornBeginManField.getNormedDate();
                    var eve = evenBeginManField.getNormedDate();

                    return eve - mor <= trTime || eve - mor >= 86400000 - trTime;
                }
                font.italic: true
                text: i18n("Error: Transition time overlaps.")
            }
        }

        // Show location chooser in manual location mode
        LocationsFixedView {
            visible: modeSwitcher.currentIndex === CC.CompositorAdaptor.ModeLocation
            enabled: activator.checked
        }

        // Show start/end times in automatic and manual location modes
        TimingsView {
            id: timings
            visible: modeSwitcher.currentIndex === CC.CompositorAdaptor.ModeAutomatic ||
                     modeSwitcher.currentIndex === CC.CompositorAdaptor.ModeLocation
            enabled: activator.checked
            latitude: modeSwitcher.currentIndex === CC.CompositorAdaptor.ModeAutomatic ? locator.latitude : cA.latitudeFixedStaged
            longitude: modeSwitcher.currentIndex === CC.CompositorAdaptor.ModeAutomatic ? locator.longitude : cA.longitudeFixedStaged
        }
    }
}
