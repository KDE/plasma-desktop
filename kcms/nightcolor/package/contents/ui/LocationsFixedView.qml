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
import org.kde.kirigami 2.5 as Kirigami
import QtQuick.Controls 2.5 as QQC2

Kirigami.FormLayout {
    twinFormLayouts: parentLayout
    enabled: activator.checked

    Connections {
        target: root
        onReset: reset()
    }

    function reset() {
        latitudeFixedField.backend = cA.latitudeFixed;
        longitudeFixedField.backend = cA.longitudeFixed;
    }

    NumberField {
        id: latitudeFixedField
        Kirigami.FormData.label: i18n("Latitude:")
        backend: cA.latitudeFixedStaged
        validator: DoubleValidator {bottom: -90; top: 90; decimals: 10}
        onBackendChanged: {
            cA.latitudeFixedStaged = backend;
            manualLocationsViewRow.change();
            calcNeedsSave();
        }
    }

    NumberField {
        id: longitudeFixedField
        Kirigami.FormData.label: i18n("Longitude:")
        backend: cA.longitudeFixedStaged
        validator: DoubleValidator {bottom: -180; top: 180; decimals: 10}
        onBackendChanged: {
            cA.longitudeFixedStaged = backend;
            manualLocationsViewRow.change();
            calcNeedsSave();
        }
    }

    QQC2.Button {
        text: i18n("Detect Location")
        implicitWidth: longitudeFixedField.width // TODO: see if there is a smarter way for doing this
        icon.name: "edit-paste-in-place"
        onClicked: {
            latitudeFixedField.backend = locator.latitude;
            longitudeFixedField.backend = locator.longitude;
        }
    }
}
