/*   vim:set foldmethod=marker:
 *
 *   Copyright (C) 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: root

    property alias showSearch   : searchButton.checked
    property alias searchString : searchText.text

    Keys.onPressed: {
        if (event.key == Qt.Key_Escape) {
            if (root.showSearch) {
                event.accepted = true;
                searchButton.checked = false;
            }
        }
    }

    onShowSearchChanged: if (showSearch) searchText.focus = true

    height : heading.height
    width  : parent.width

    PlasmaComponents.ToolButton {
        id: searchButton
        iconSource: "edit-find"

        checkable: true
        checked: root.showSearch

        anchors {
            right  : parent.right
            top    : parent.top
            bottom : parent.bottom
        }

        onCheckedChanged: {
            if (!checked) searchText.text = "";
        }
    }

    PlasmaExtras.Title {
        id: heading
        text: "Activities"

        anchors {
            left   : parent.left
            right  : searchButton.left
            bottom : parent.bottom
            top    : parent.top
        }
    }

    PlasmaComponents.TextField {
        id: searchText

        focus: true

        anchors {
            left   : parent.left
            right  : searchButton.left
            bottom : parent.bottom
            top    : parent.top
        }

        onTextChanged: {
            if (text.length > 0) {
                root.showSearch = true
            }
        }
    }

    states: [
        State {
            name: "normal"
            PropertyChanges { target: searchText; opacity: 0 }
            PropertyChanges { target: heading; opacity: 1 }
        },
        State {
            name: "search"
            PropertyChanges { target: searchText; opacity: 1 }
            PropertyChanges { target: heading; opacity: 0 }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {
                properties : "opacity"
                duration   : units.shortDuration
            }
        }
    ]

    state: showSearch ? "search" : "normal"
}
