// Copyright (C) 2019 Julian Sherollari <jdotsh@gmail.com>
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtPositioning
import QtLocation
import Qt.labs.qmlmodels 1.0

DelegateChooser {
    id: dc
    role: "type"
    property color defaultColor: "#46a2da"
    property real defaultOpacity: 0.6

    DelegateChoice {
        roleValue: "Polygon"
        delegate: MapPolygon {
            property string geojsonType: "Polygon"
            property string tzid: modelData?.properties?.tzid || ""
            property string ptzid: parent.tzid || ""
            property string atzid: tzid || ptzid
            property bool tzidhover: win.hoveredTimeZone == atzid
            geoShape: modelData.data
            opacity: tzidhover ? dc.defaultOpacity : 0.1
            color: dc.defaultColor
            border.width: 2
            border.color: Qt.darker(color)
            referenceSurface: view.referenceSurface

            TapHandler {
                onTapped: {}
            }
            HoverHandler {
                id: hh
                enabled: !!tzid || !!ptzid
                onHoveredChanged: {
                    if (hovered) {
                        win.hoveredTimeZone = atzid
                        console.log(win.hoveredTimeZone)
                    } else if (win.hoveredTimeZone == atzid) {
                        win.hoveredTimeZone = ""
                    }
                }
            }
        }
    }

    DelegateChoice {
        roleValue: "MultiPolygon"
        delegate: MapItemView {
            property string geojsonType: "MultiPolygon"
            property string tzid: modelData?.properties?.tzid
            model: modelData.data
            delegate: dc
        }
    }

    DelegateChoice {
        roleValue: "FeatureCollection"
        delegate: MapItemView {
            property string geojsonType: "FeatureCollection"
            model: modelData.data
            delegate: dc
        }
    }
}
