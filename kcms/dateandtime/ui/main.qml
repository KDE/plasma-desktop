/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import QtQuick.Controls

import QtPositioning
import QtLocation
import QtCore

import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami
import org.kde.plasma.components 3.0 as PC3

KCM.SimpleKCM {
    id: win

    property string hoveredTimeZone: ""
    property string timezoneFileUrl: ""

    GeoJsonData {
        id: geoDatabase
        // GeoJsonData does not support qrc paths, so we have to install
        // the timezones file into the shared folder.
        sourceUrl: "file:/" + DTime.timezoneDataPath() + "/timezones.json"
    }

    MapView {
        id: view

        anchors.fill: parent

        map.plugin: Plugin {
            name: "osm"
            PluginParameter {
                name: 'osm.mapping.offline.directory'
                value: ":/kcm/kcm_clock/offline_tiles"
            }
        }
        map.zoomLevel: 0
        map.minimumZoomLevel: 0
        map.maximumZoomLevel: 4
        map.activeMapType: map.supportedMapTypes[0]

        property variant referenceSurface: QtLocation.ReferenceSurface.Map

        MapItemView {
            id: miv
            parent: view.map
            model: geoDatabase.model
            delegate: GeoJsonDelegate {}
        }

        Kirigami.ShadowedRectangle {
            color: Kirigami.Theme.backgroundColor

            shadow {
                size: Kirigami.Units.gridUnit * 2
                color: Qt.rgba(0, 0, 0, 0.3)
                yOffset: 3
            }

            radius: Kirigami.Units.cornerRadius

            anchors.bottom: parent.bottom
            anchors.bottomMargin: Kirigami.Units.gridUnit
            anchors.horizontalCenter: parent.horizontalCenter

            width: mainLayout.implicitWidth + Kirigami.Units.gridUnit
            height: mainLayout.implicitHeight + Kirigami.Units.gridUnit

            ColumnLayout {
                id: mainLayout
                anchors.centerIn: parent
                spacing: Kirigami.Units.smallSpacing

                RowLayout {

                    PC3.Label {
                        text: i18nc("Beginning of the sentence 'You have currently selected the [Rome/Dublin/..] area in the [Europe/America/…] region.', in the context of timezones.", "You have currently selected the")
                    }

                    QQC2.ComboBox {
                        id: regionComboBox
                        model: DTime.availableTimeZoneRegions()

                        Connections {
                            target: DTime
                            function onSelectedTimeZoneChanged() {
                                regionComboBox.currentIndex = regionComboBox.model.indexOf(DTime.selectedTimeZone.split('/')[0])
                            }
                        }

                        popup.onClosed: {
                            if (regionComboBox.currentText !== DTime.selectedTimeZone.split('/')[0]) {
                                let locations = DTime.availableTimeZoneLocationsInRegion(regionComboBox.currentText)
                                if (locations.length > 0) {
                                    locationComboBox.model = locations
                                    locationComboBox.popup.visible = true
                                } else {
                                    locationComboBox.model = []
                                    DTime.selectedTimeZone = regionComboBox.currentText
                                }
                            }
                        }
                    }

                    PC3.Label {
                        text: locationComboBox.visible ? i18nc("Part of the sentence 'You have currently selected the [Rome/Dublin/..] area in the [Europe/America/…] region.', in the context of timezones.", "area in the") : i18nc("End of the sentence 'You have currently selected the [UTC+0/UTC+1/..] timezone.'.", "You have currently selected the")
                    }

                    QQC2.ComboBox {
                        id: locationComboBox

                        visible: locationComboBox.model.length > 0

                        Connections {
                            target: DTime
                            function onSelectedTimeZoneChanged() {
                                locationComboBox.model = DTime.availableTimeZoneLocationsInRegion(DTime.selectedTimeZone.split('/')[0])
                                locationComboBox.currentIndex = locationComboBox.model.indexOf(DTime.selectedTimeZone.split('/')[1])
                            }
                        }

                        popup.onClosed: {
                            DTime.selectedTimeZone = regionComboBox.currentText + '/' + locationComboBox.currentText
                        }
                    }

                    PC3.Label {
                        text: i18nc("End of the sentence 'You have currently selected the [Rome/Dublin/..] area in the [Europe/America/…] region.', in the context of timezones.", "region.")
                        visible: locationComboBox.visible
                    }
                }
            }
        }
    }

}
