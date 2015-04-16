/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Layouts 1.0
import org.kde.kquickcontrols 2.0

Item {
    id: root
    width: childrenRect.width
    height: childrenRect.height

    signal configurationChanged
    function saveConfig() {
        plasmoid.globalShortcut = button.keySequence
    }

    ColumnLayout {
        anchors {
            left: parent.left
            right: parent.right
        }
        QtControls.Label {
            Layout.fillWidth: true
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "This shortcut will activate the applet: it will give the keyboard focus to it, and if the applet has a popup (such as the start menu), the popup will be open.")
            wrapMode: Text.WordWrap
        }
        KeySequenceItem {
            id: button
            keySequence: plasmoid.globalShortcut
            onKeySequenceChanged: {
                root.configurationChanged();
            }
        }
    }
}
