/*
 *  Copyright 2015 Marco Martin <mart@kde.org>
 *  Copyright 2020 Nicolas Fella <nicolas.fella@gmx.de>
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
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: page

    property QtObject kcm

    signal settingValueChanged()

    function saveConfig() {
        kcm.save()
    }

    onKcmChanged: {
        kcm.mainUi.parent = page
        kcm.mainUi.anchors.fill = page
        kcm.load()
    }

    Connections {
        target: kcm
        function onNeedsSaveChanged() {
            if (kcm.needsSave) {
                page.settingValueChanged()
            }
        }
    }
}
