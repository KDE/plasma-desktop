// Copyright (C) 2019 Julian Sherollari <jdotsh@gmail.com>
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtPositioning
import QtLocation
import Qt.labs.qmlmodels 1.0

import org.kde.kirigami as Kirigami

DelegateChooser {
    id: dc
    role: "type"

    property color defaultColor: Kirigami.Theme.highlightColor
    property real selectedOpacity: 1
    property real hoveredOpacity: 0.6
    property real defaultOpacity: 0.1

    DelegateChoice {
        roleValue: "Polygon"
        delegate: MapPolygon {
            property string tzid: modelData?.properties?.tzid || parent.tzid || ""
            property bool tzidhover: win.hoveredTimeZone == tzid
            property bool tzidselected: win.selectedTimeZone == tzid
            geoShape: modelData.data
            opacity: tzidselected ? selectedOpacity : (tzidhover ? dc.hoveredOpacity : dc.defaultOpacity)
            color: dc.defaultColor
            border.width: 2
            border.color: Qt.darker(color)
            referenceSurface: view.referenceSurface

            TapHandler {
                onTapped: {
                    win.selectedTimeZone = tzid
                }
            }

            HoverHandler {
                id: hh
                enabled: !!tzid
                onHoveredChanged: {
                    if (hovered) {
                        win.hoveredTimeZone = tzid
                    } else if (win.hoveredTimeZone == tzid) {
                        win.hoveredTimeZone = ""
                    }
                }
            }

        }
    }

    DelegateChoice {
        roleValue: "MultiPolygon"
        delegate: MapItemView {
            property string tzid: modelData?.properties?.tzid
            model: modelData.data
            delegate: dc
        }
    }

    DelegateChoice {
        roleValue: "FeatureCollection"
        delegate: MapItemView {
            model: modelData.data
            delegate: dc
        }
    }
}
