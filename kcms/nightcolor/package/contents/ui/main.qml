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

        Kirigami.FormLayout {
            id: parentLayout

            QQC2.CheckBox {
                id: activator
                text: i18n("Activate Night Color")
                enabled: cA.nightColorAvailable
                checked: cA.active

                onCheckedChanged: {
                    cA.activeStaged = checked;
                    calcNeedsSave();
                }
            }

            RowLayout {
                Kirigami.FormData.label: i18n("Night Color temperature:")
                enabled: activator.checked

                QQC2.Slider {
                    id: tempSlider
                    enabled: activator.checked
                    from: cA.minimalTemperature
                    implicitWidth: modeSwitcher.width
                    to: cA.neutralTemperature
                    value: cA.nightTemperature
                    stepSize: 100

                    onValueChanged: {
                        cA.nightTemperatureStaged = value;
                        calcNeedsSave();
                    }
                }

                    QQC2.Label {
                        text: tempSlider.value + i18n(" K")
                    }
                }

            QQC2.ComboBox {
                id: modeSwitcher
                Kirigami.FormData.label: i18n("Operation mode:")
                enabled: activator.checked
                model: [
                    i18n("Automatic"),
                    i18n("Location"),
                    i18n("Times"),
                    i18n("Constant")
                ]
                currentIndex: cA.mode
                onCurrentIndexChanged: {
                    cA.modeStaged = currentIndex;
                    advancedControlLoader.updatePage(currentIndex);
                    calcNeedsSave();
                }
            }
        }

        Kirigami.FormLayout {

            Loader {
                id: advancedControlLoader


                function updatePage(index) {
                    switch (index) {
                    case CC.CompositorAdaptor.ModeAutomatic:
                        sourceComponent = automaticView;
                        break;
                    case CC.CompositorAdaptor.ModeLocation:
                        sourceComponent = manualLocationsView;
                        break;
                    case CC.CompositorAdaptor.ModeTimings:
                        sourceComponent = manualTimingsView;
                        break;
                    case CC.CompositorAdaptor.ModeConstant:
                    default:
                        sourceComponent = undefined;
                        break;
                    }
                }
            }

            Component {
                id: automaticView

                    ColumnLayout {

                    Loader {
                        sourceComponent: TimingsView {
                            latitude: locator.latitude
                            longitude: locator.longitude
                        }
                    }

                    Kirigami.Separator {
                        Layout.fillWidth: true
                        Kirigami.FormData.isSection: true
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

                ColumnLayout {
                    id: manualLocationsViewRow
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

                    Kirigami.Separator {
                        Layout.fillWidth: true
                        Kirigami.FormData.isSection: true
                    }

                    Loader {
                        sourceComponent: LocationsFixedView {}
                    }
                }
            }

            Component {
                id: manualTimingsView

                ColumnLayout {
                    Loader {
                        sourceComponent: Kirigami.FormLayout {
                            twinFormLayouts: parentLayout
                            enabled: activator.checked && cA.timingsEnabled

                            Connections {
                                target: root
                                onReset: {
                                    mornBeginManField.backend = cA.morningBeginFixed;
                                    evenBeginManField.backend = cA.eveningBeginFixed;
                                    transTimeField.backend = cA.transitionTime;
                                }
                            }

                            TimeField {
                                id: mornBeginManField
                                Kirigami.FormData.label: i18n("Sunrise begins:")
                                backend: cA.morningBeginFixedStaged
                                onBackendChanged: {cA.morningBeginFixedStaged = backend;
                                    calcNeedsSave();
                                }

                                QQC2.ToolTip {
                                    text: i18n("(Input format: HH:MM)")
                                }
                            }

                            TimeField {
                                id: evenBeginManField
                                Kirigami.FormData.label: i18n("Sunset begins:")
                                backend: cA.eveningBeginFixedStaged
                                onBackendChanged: {cA.eveningBeginFixedStaged = backend;
                                    calcNeedsSave();
                                }

                                QQC2.ToolTip {
                                    text: i18n("Input format: HH:MM")
                                }
                            }

                            NumberField {
                                id: transTimeField
                                Kirigami.FormData.label: i18n("Transition duration:")
                                backend: cA.transitionTimeStaged
                                onBackendChanged: {cA.transitionTimeStaged = backend;
                                    calcNeedsSave();
                                }

                                inputMethodHints: Qt.ImhDigitsOnly
                                validator: IntValidator {bottom: 1; top: 600;}  // less than 12 hours (in minutes: 720)

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
                    }
                }
            }
        }
    }
}
