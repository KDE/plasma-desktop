/*   vim:set foldmethod=marker:
 *
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic@kde.org>
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
import QtQuick.Controls 1.0 as QtControls

import "./components" as Local

Item {
    id: root

    property alias activityIsPrivate : checkPrivate.checked
    property alias activityShortcut  : panelShortcut.keySequence

    height : content.childrenRect.height + 4 * units.smallSpacing
    width  : content.childrenRect.width + 4 * units.smallSpacing

    Column {
        anchors {
            fill: parent
            margins: 2 * units.smallSpacing
        }

        QtControls.CheckBox {
            id: checkPrivate

            text: i18nd("kcm_activities5", "Private - do not track usage for this activity")

            width: parent.width
        }

        Local.ShortcutChooser {
            id: panelShortcut

            width: parent.width
        }
    }
}
