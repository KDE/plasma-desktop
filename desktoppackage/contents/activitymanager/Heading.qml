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

    property alias searchString : searchText.text
    signal closeRequested

    function focusSearch() {
        searchText.forceActiveFocus()
    }

    Keys.onPressed: {
        if (event.key == Qt.Key_Escape) {
            if (root.showSearch) {
                event.accepted = true;
                searchButton.checked = false;
            }
        }
    }

    height: childrenRect.height

    PlasmaExtras.Title {
        id: heading
        text: i18nd("plasma_shell_org.kde.plasma.desktop", "Activities")
        elide: Text.ElideRight

        anchors {
            left   : parent.left
            right  : closeButton.left
            rightMargin: units.smallSpacing
            top    : parent.top
        }
    }

    PlasmaComponents.ToolButton {
        id: closeButton
        anchors {
            right: parent.right
            verticalCenter: heading.verticalCenter
        }
        iconSource: "window-close"
        onClicked: root.closeRequested()
    }


    PlasmaComponents.TextField {
        id: searchText

        focus: true
        clearButtonShown: true

        anchors {
            left   : parent.left
            right  : parent.right
            top    : heading.bottom
        }

        placeholderText: i18nd("plasma_shell_org.kde.plasma.desktop", "Search...")
    }
}
