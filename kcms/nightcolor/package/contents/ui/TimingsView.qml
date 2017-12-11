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

    property double latitude
    property double longitude

    property var morningTimings: sunCalc.getMorningTimings(latitude, longitude)
    property var eveningTimings: sunCalc.getEveningTimings(latitude, longitude)

    function reset() {
        morningTimings = sunCalc.getMorningTimings(latitude, longitude);
        eveningTimings = sunCalc.getEveningTimings(latitude, longitude);
    }

    Controls.Label {
        text: i18n("Sunrise begins")
        Layout.alignment: Qt.AlignRight
        enabled: false
    }
    TimeField {
        id: mornBeginField
        backend: morningTimings.begin
        enabled: false
    }
    Controls.Label {
        text: i18n("and ends")
        Layout.alignment: Qt.AlignRight
        enabled: false
    }
    TimeField {
        id: mornEndField
        backend: morningTimings.end
        enabled: false
    }
    Controls.Label {
        text: i18n("Sunset begins")
        Layout.alignment: Qt.AlignRight
        enabled: false
    }
    TimeField {
        id: evenBeginField
        backend: eveningTimings.begin
        enabled: false
    }
    Controls.Label {
        text: i18n("and ends")
        Layout.alignment: Qt.AlignRight
        enabled: false
    }
    TimeField {
        id: evenEndField
        backend: eveningTimings.end
        enabled: false
    }
}
