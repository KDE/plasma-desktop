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
    property double backend

    maximumLength: 10
    horizontalAlignment: TextInput.AlignHCenter

    inputMethodHints: Qt.ImhFormattedNumbersOnly

    text: backend

    onBackendChanged: {
        text = backend;
    }

    onTextChanged: {
        var textFloat = parseFloat(text);
        if (textFloat === undefined || isNaN(textFloat)) {
            return;
        }
        backend = textFloat;
    }

    onFocusChanged: {
        var textFloat = parseFloat(text);
        if (!focus && (textFloat === undefined || isNaN(textFloat))) {
            text = backend;
        }
    }
}
