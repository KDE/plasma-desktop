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

    property double latitude
    property double longitude

    property var morningTimings: sunCalc.getMorningTimings(latitude, longitude)
    property var eveningTimings: sunCalc.getEveningTimings(latitude, longitude)

    function reset() {
        morningTimings = sunCalc.getMorningTimings(latitude, longitude);
        eveningTimings = sunCalc.getEveningTimings(latitude, longitude);
    }

    TimeField {
        id: mornBeginField
        Kirigami.FormData.label: i18n("Sunrise begins:")
        backend: morningTimings.begin
        enabled: false
    }

    TimeField {
        id: mornEndField
        Kirigami.FormData.label: i18n("...and ends:")
        backend: morningTimings.end
        enabled: false
    }

    TimeField {
        id: evenBeginField
        Kirigami.FormData.label: i18n("Sunset begins:")
        backend: eveningTimings.begin
        enabled: false
    }

    TimeField {
        id: evenEndField
        Kirigami.FormData.label: i18n("...and ends:")
        backend: eveningTimings.end
        enabled: false
    }
}
