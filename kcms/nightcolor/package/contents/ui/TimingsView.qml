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

    property double latitude
    property double longitude

    property var morningTimings: sunCalc.getMorningTimings(latitude, longitude)
    property var eveningTimings: sunCalc.getEveningTimings(latitude, longitude)

    function reset() {
        morningTimings = sunCalc.getMorningTimings(latitude, longitude);
        eveningTimings = sunCalc.getEveningTimings(latitude, longitude);
    }

    function prettyTime(date) {
        return date.toLocaleString(Qt.locale(), "h:mm");
    }

    Kirigami.Separator {
        Kirigami.FormData.isSection: true
    }

    QQC2.Label {
        wrapMode: Text.Wrap
        text: i18n("Night Color begins at %1", prettyTime(eveningTimings.begin))
    }
    QQC2.Label {
        wrapMode: Text.Wrap
        text: i18n("Color fully changed at %1", prettyTime(eveningTimings.end))
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    QQC2.Label {
        wrapMode: Text.Wrap
        text: i18n("Night Color begins changing back at %1", prettyTime(morningTimings.begin))
    }
    QQC2.Label {
        wrapMode: Text.Wrap
        text: i18n("Normal coloration restored by %1", prettyTime(morningTimings.end))
    }
}
