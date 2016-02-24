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

import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    property alias text  : textField.text
    property alias label : label.text

    property alias labelWidth        : label.width
    property alias desiredLabelWidth : label.contentWidth

    height : textField.height
    width  : parent.width

    QtControls.Label {
        id: label

        horizontalAlignment: Text.AlignRight

        anchors {
            left: parent.left
            verticalCenter: textField.verticalCenter
        }
    }

    QtControls.TextField {
        id: textField

        anchors {
            left  : label.right
            right : parent.right

            leftMargin : units.smallSpacing
        }
    }
}
