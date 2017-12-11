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

GridLayout {
    columns: 2
    rowSpacing: units.smallSpacing
    columnSpacing: units.smallSpacing
    enabled: activator.checked

    Connections {
        target: root
        onReset: reset()
    }

    function reset() {
        latitudeFixedField.backend = cA.latitudeFixed;
        longitudeFixedField.backend = cA.longitudeFixed;
    }

    Controls.Label {
        text: i18n("Latitude")
        Layout.alignment: Qt.AlignRight
    }
    NumberField {
        id: latitudeFixedField
        backend: cA.latitudeFixedStaged
        validator: DoubleValidator {bottom: -90; top: 90; decimals: 10}
        onBackendChanged: {
            cA.latitudeFixedStaged = backend;
            manualLocationsViewRow.change();
            calcNeedsSave();
        }
    }

    Controls.Label {
        text: i18n("Longitude")
        Layout.alignment: Qt.AlignRight
    }
    NumberField {
        id: longitudeFixedField
        backend: cA.longitudeFixedStaged
        validator: DoubleValidator {bottom: -180; top: 180; decimals: 10}
        onBackendChanged: {
            cA.longitudeFixedStaged = backend;
            manualLocationsViewRow.change();
            calcNeedsSave();
        }
    }
    Controls.Button {
        Layout.columnSpan: 2
        Layout.alignment: Qt.AlignHCenter
        text: i18n("Detect location")
        onClicked: {
            latitudeFixedField.backend = locator.latitude;
            longitudeFixedField.backend = locator.longitude;
        }
    }
}
