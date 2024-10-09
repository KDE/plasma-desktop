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

import org.kde.plasma.timezone.kcm

KCM.SimpleKCM {
    id: win

    property string hoveredTimeZone

    GeoJsonData {
        id: geoDatabase
        sourceUrl: "file:/home/niccolove/kde/src/plasma-desktop/kcms/timezone/ui/timezones.json"
    }

    MapView {
        id: view

        anchors {
            left: parent.left
            top: parent.top
            right: parent.right
        }
        height: Kirigami.Units.gridUnit * 25

        map.plugin: Plugin {
            name: "osm"
            PluginParameter {
                name: 'osm.mapping.offline.directory'
                value: '/home/niccolove/kde/src/plasma-desktop/kcms/timezone/ui/offline_tiles_third/'
            }
        }
        map.zoomLevel: 0
        map.minimumZoomLevel: 0
        map.maximumZoomLevel: 5
        map.center: QtPositioning.coordinate(3, 8)
        map.activeMapType: map.supportedMapTypes[0]

        property variant referenceSurface: QtLocation.ReferenceSurface.Map

        MapItemView {
            id: miv
            parent: view.map
            model: geoDatabase.model
            delegate: GeoJsonDelegate {}
        }
    }

    // Labels and buttons will go here

}
