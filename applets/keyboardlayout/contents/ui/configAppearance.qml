/*
*  Copyright 2018 Aleix Pol Gonzalez <aleixpol@kde.org>
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

import QtQuick 2.2
import QtQuick.Controls 2.2 as Controls
import QtQuick.Layouts 1.1

Item
{
    id: root
    signal configurationChanged

    function saveConfig() {
        var names = []
        for(var i in layout.children) {
            var cur = layout.children[i]
            if (cur.checked)
                names.push(cur.name)
        }
        plasmoid.configuration.key = names
    }

    ColumnLayout {
        id: layout
        Controls.CheckBox {
            Layout.fillWidth: true
            readonly property string name: "Caps Lock"
            checked: plasmoid.configuration.key.indexOf(name) >= 0
            text: i18nc("@option:check", "Caps Lock")
            onCheckedChanged: root.configurationChanged()
        }
        Controls.CheckBox {
            Layout.fillWidth: true
            readonly property string name: "Num Lock"
            checked: plasmoid.configuration.key.indexOf(name) >= 0
            text: i18nc("@option:check", "Num Lock")
            onCheckedChanged: root.configurationChanged()
        }
    }
}
