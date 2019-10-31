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
import QtQuick.Controls 2.5 as QQC2

QQC2.TextField {
    id: field

    property date backend
    horizontalAlignment: TextInput.AlignHCenter

    onBackendChanged: readIn()

    function getNormedDate() {
        var nD = new Date();
        nD.setHours(backend.getHours());
        nD.setMinutes(backend.getMinutes());
        return nD;
    }

    function readIn() {
        if (!backend) {
            return;
        }

        var hours = backend.getHours();
        var minutes = backend.getMinutes();
        if (hours < 10) {
            hours = "0" + hours;
        }
        if (minutes < 10) {
            minutes = "0" + minutes;
        }

        text = hours + ":" + minutes;
    }

    function submit() {
        if (text.length != 5) {
            return;
        }
        var hours = text.slice(0, 2);
        var minutes = text.slice(3, 5);

        var date = new Date();
        date.setHours(hours, minutes, 0, 0);

        backend = date;
    }

    onTextChanged: submit()
    inputMask: "00:00"
    selectByMouse: false
    inputMethodHints: Qt.ImhPreferNumbers
    validator: RegExpValidator { regExp: /^([0-1]?[0-9]|2[0-3]):[0-5][0-9]$/ }

    onEditingFinished: submit()
}
