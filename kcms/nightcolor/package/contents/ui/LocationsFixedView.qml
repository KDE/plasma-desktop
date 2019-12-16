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
import QtQuick.Controls 2.5 as QQC2
import org.kde.kirigami 2.5 as Kirigami

Kirigami.FormLayout {
    twinFormLayouts: parentLayout

    Connections {
        target: root
        onReset: reset()
        onDefaults: {
            latitudeFixedField.backend = cA.latitudeFixedDefault;
            longitudeFixedField.backend = cA.longitudeFixedDefault;
        }
    }

    function reset() {
        latitudeFixedField.backend = cA.latitudeFixed;
        longitudeFixedField.backend = cA.longitudeFixed;
    }

    QQC2.Button {
        text: i18n("Detect Location")
        // Match combobox width
        Layout.minimumWidth: modeSwitcher.width
        icon.name: "find-location"
        onClicked: {
            latitudeFixedField.backend = locator.latitude;
            longitudeFixedField.backend = locator.longitude;
        }
    }

    NumberField {
        id: latitudeFixedField
        // Match combobox width
        Layout.minimumWidth: modeSwitcher.width
        Kirigami.FormData.label: i18n("Latitude:")
        backend: cA.latitudeFixedStaged
        validator: DoubleValidator {bottom: -90; top: 90; decimals: 10}
        onBackendChanged: {
            cA.latitudeFixedStaged = backend;
            calcNeedsSave();
        }
    }

    NumberField {
        id: longitudeFixedField
        // Match combobox width
        Layout.minimumWidth: modeSwitcher.width
        Kirigami.FormData.label: i18n("Longitude:")
        backend: cA.longitudeFixedStaged
        validator: DoubleValidator {bottom: -180; top: 180; decimals: 10}
        onBackendChanged: {
            cA.longitudeFixedStaged = backend;
            calcNeedsSave();
        }
    }
}
