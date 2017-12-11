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
import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4 as Controls

import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.colorcorrect 0.1 as CC

Item {
    id: root

    implicitHeight: units.gridUnit * 20

    property int error: cA.error

    property bool defaultRequested: false

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
    }

    Connections {
        target: kcm
        onLoadRelay: cA.reloadData()
        onSaveRelay: defaultRequested ? cA.sendConfigurationAll() : cA.sendConfiguration();
        onDefaultsRelay: {
            if (!cA.nightColorAvailable) {
                return;
            }
            activator.checked = cA.activeDefault;
            tempSlider.value = cA.nightTemperatureDefault;
            modeSwitcher.currentIndex = cA.modeDefault;

            // set everything else to default as well
            cA.latitudeFixedStaged = cA.latitudeFixedDefault;
            cA.longitudeFixedStaged = cA.longitudeFixedDefault;

            cA.morningBeginFixedStaged = cA.morningBeginFixedDefault;
            cA.eveningBeginFixedStaged = cA.eveningBeginFixedDefault;
            cA.transitionTimeStaged = cA.transitionTimeDefault;

            kcm.needsSave = cA.checkStagedAll();
            defaultRequested = true;
        }
    }

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

    Item {
        id: main
        width: childrenRect.width
        height: childrenRect.height

        enabled: cA.nightColorAvailable

        Controls.CheckBox {
            id: activator
            text: i18n("Activate Night Color")
            checked: cA.active

            onCheckedChanged: {
                cA.activeStaged = checked;
                calcNeedsSave();
            }
        }

        ColumnLayout {
            id: mainControls
            anchors.top: activator.bottom
            anchors.topMargin: units.largeSpacing

            enabled: activator.checked

            Controls.Label {
                text: i18n("Night Color Temperature: ") + tempSlider.value + i18n(" K")
            }

            Controls.Slider {
                id: tempSlider
                implicitWidth: units.gridUnit * 15

                minimumValue: cA.minimalTemperature
                maximumValue: cA.neutralTemperature
                value: cA.nightTemperature
                stepSize: 100

                onValueChanged: {
                    cA.nightTemperatureStaged = value;
                    calcNeedsSave();
                }
            }

            RowLayout {
                spacing: units.largeSpacing
                Controls.Label {
                    text: i18n("Operation mode:")
                }
                Controls.ComboBox {
                    id: modeSwitcher
                    width: theme.mSize(theme.defaultFont).width * 9
                    model: [i18n("Automatic"),
                    i18n("Location"),
                    i18n("Times")]
                    currentIndex: cA.mode
                    onCurrentIndexChanged: {
                        cA.modeStaged = currentIndex;
                        advancedControlLoader.updatePage(currentIndex);
                        calcNeedsSave();
                    }
                }
            }
        }

        Loader {
            id: advancedControlLoader

            anchors.top: mainControls.bottom
            anchors.topMargin: units.largeSpacing

            function updatePage(index) {
                sourceComponent = undefined;
                var page;
                if (index == CC.CompositorAdaptor.ModeLocation) {
                    page = manualLocationsView;
                } else if (index == CC.CompositorAdaptor.ModeTimings) {
                    page = manualTimingsView;
                } else {
                    page = automaticView;
                }

                sourceComponent = page;
            }
        }

        Component {
            id: automaticView
            Row {
                spacing: units.largeSpacing

                Loader {
                    sourceComponent: TimingsView {
                        latitude: locator.latitude
                        longitude: locator.longitude
                    }
                }
                Loader {
                    sourceComponent: LocationsAutoView {
                        latitude: locator.latitude
                        longitude: locator.longitude
                    }
                }
            }
        }

        Component {
            id: manualLocationsView

            Row {
                id: manualLocationsViewRow
                spacing: units.largeSpacing

                signal change()

                Loader {
                    sourceComponent: TimingsView {
                        latitude: cA.latitudeFixedStaged
                        longitude: cA.longitudeFixedStaged

                        Connections {
                            target: manualLocationsViewRow
                            onChange: {
                                reset();
                            }
                        }
                    }
                }
                Loader {
                    sourceComponent: LocationsFixedView {}
                }
            }
        }

        Component {
            id: manualTimingsView
            Column {
                spacing: units.smallSpacing

                GridLayout {
                    id: manualTimingsViewGrid

                    columns: 3
                    rowSpacing: units.smallSpacing
                    columnSpacing: units.smallSpacing
                    enabled: activator.checked && cA.timingsEnabled

                    Connections {
                        target: root
                        onReset: {
                            mornBeginManField.backend = cA.morningBeginFixed;
                            evenBeginManField.backend = cA.eveningBeginFixed;
                            transTimeField.backend = cA.transitionTime;
                        }
                    }

                    Controls.Label {
                        text: i18n("Sunrise begins")
                        Layout.alignment: Qt.AlignRight
                    }
                    TimeField {
                        id: mornBeginManField
                        backend: cA.morningBeginFixedStaged
                        onBackendChanged: {cA.morningBeginFixedStaged = backend;
                            calcNeedsSave();
                        }
                    }
                    Controls.Label {
                        enabled: false
                        text: i18n("(HH:MM)")
                    }
                    Controls.Label {
                        text: i18n("Sunset begins")
                        Layout.alignment: Qt.AlignRight
                    }
                    TimeField {
                        id: evenBeginManField
                        backend: cA.eveningBeginFixedStaged
                        onBackendChanged: {cA.eveningBeginFixedStaged = backend;
                            calcNeedsSave();
                        }
                    }
                    Controls.Label {
                        enabled: false
                        text: i18n("(HH:MM)")
                    }
                    Controls.Label {
                        text: i18n("Transition duration")
                        Layout.alignment: Qt.AlignRight
                    }
                    NumberField {
                        id: transTimeField
                        backend: cA.transitionTimeStaged
                        onBackendChanged: {cA.transitionTimeStaged = backend;
                            calcNeedsSave();
                        }

                        inputMethodHints: Qt.ImhDigitsOnly
                        validator: IntValidator {bottom: 1; top: 600;}  // less than 12 hours (in minutes: 720)
                    }
                    Controls.Label {
                        enabled: false
                        text: i18n("(In minutes - min. 1, max. 600)")
                    }
                }
                Controls.Label {
                    id: manualTimingsError1
                    anchors.horizontalCenter: manualTimingsViewGrid.horizontalCenter
                    visible: evenBeginManField.getNormedDate() - mornBeginManField.getNormedDate() <= 0

                    font.italic: true
                    text: i18n("Error: Morning not before evening.")
                }
                Controls.Label {
                    id: manualTimingsError2
                    anchors.horizontalCenter: manualTimingsViewGrid.horizontalCenter
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
        }
    }

    // error message as overlay
    Item {
        width: 0.8 * main.width
        height: 0.8 * main.height
        anchors.centerIn: main

        visible: error != CC.CompositorAdaptor.ErrorCodeSuccess

        Rectangle {
            anchors.centerIn: parent
            width: errorMessage.contentWidth * 1.1
            height: errorMessage.contentHeight * 1.1
            color: theme.backgroundColor
            opacity: 0.8

            border {
                width: units.devicePixelRatio
                color: theme.textColor
            }
        }

        PlasmaExtras.Heading {
            id: errorMessage
            anchors.fill: parent

            level: 4
            text: cA.errorText

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            wrapMode: Text.Wrap
            textFormat: Text.PlainText
        }
    }
}
