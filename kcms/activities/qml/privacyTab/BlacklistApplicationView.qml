/*   vim:set foldenable foldmethod=marker:
 *
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic@kde.org>
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

import QtQuick 2.0
import org.kde.kquickcontrolsaddons 2.0
import QtQuick.Controls 1.0 as QtControls

QtControls.ScrollView {
    anchors.fill: parent

    Flow {
        id: main

        SystemPalette {
            id: colors
        }

        width: parent.parent.width

        property int minimumHeight: 100

        spacing: 16

        height: Math.max(childrenRect.height, minimumHeight)

        opacity: applicationModel.enabled ? 1 : .3
        Behavior on opacity { NumberAnimation { duration: 150 } }

        Repeater {
            model: applicationModel
            Column {
                id: item

                property bool blocked: model.blocked

                Item {
                    id: mainIcon

                    width  : 64 + 20
                    height : 64 + 20

                    QIconItem {
                        id: icon
                        icon: model.icon

                        anchors.fill    : parent
                        anchors.margins : 10

                        opacity: item.blocked ? 0.5 : 1.0
                        Behavior on opacity { NumberAnimation { duration: 150 } }
                    }

                    QIconItem {
                        id: iconNo
                        icon: "dialog-cancel"

                        anchors {
                            right  : parent.right
                            bottom : parent.bottom
                        }

                        width   : 48
                        height  : 48
                        opacity : (1 - icon.opacity) * 2
                    }

                    MouseArea {
                        onClicked: applicationModel.toggleApplicationBlocked(model.index)
                        anchors.fill: parent
                    }
                }

                Text {
                    elide   : Text.ElideRight
                    width   : mainIcon.width

                    text    : model.title
                    opacity : icon.opacity
                    color   : colors.windowText

                    anchors.margins          : 10
                    anchors.horizontalCenter : parent.horizontalCenter
                    horizontalAlignment      : Text.AlignHCenter
                }
            }
        }
    }

}

