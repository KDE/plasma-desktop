/*
    Copyright 2019 Harald Sitter <sitter@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5

import org.kde.tastenbrett.private 1.0 as XKB

Window {
    id: window
    visible: false
    width: 1024
    height: 768

    SystemPalette { id: activePalette; colorGroup: SystemPalette.Active }
    SystemPalette { id: disabledPalette; colorGroup: SystemPalette.Disabled }

    function createDoodad(doodad, parent, properties) {
        var component = null
        if (doodad instanceof XKB.LogoDoodad) {
            component = doodadLogoComponent
        } else if (doodad instanceof XKB.IndicatorDoodad) {
            component = doodadIndicatorComponent
        } else if (doodad instanceof XKB.TextDoodad) {
            component = doodadTextComponent
        } else if (doodad instanceof XKB.ShapeDoodad) {
            component = doodadShapeComponent
        }
        if (component) {
            return component.createObject(parent, properties)
        }
        console.error("Unknown doodad type!", doodad)
        return null
    }

    Rectangle {
        anchors.fill: parent
        color: activePalette.window
    }

    Item {
        id: kbd
        width: window.width
        height: window.height

        // XKB geometries have a consistent coordinate system within a geometry
        // all children follow that system instead of QML. To easily convert
        // everything wholesale we'll employ a scale factor.
        property real childWidth: childrenRect.x + childrenRect.width
        property real childHeight: childrenRect.y + childrenRect.height
        // We preserve the ratio and as a result will need to scall either
        // with height or width, whichever has less space.
        scale: Math.min(width / childWidth, height / childHeight)
        transformOrigin: Item.TopLeft
    }

    Component {
        id: keyComponent

        // Interactable Key
        Key {
            id: root

            MouseArea {
                id: hoverArea
                anchors.fill: parent
                hoverEnabled: true
            }

            ToolTip {
                visible: hoverArea.containsMouse

                contentItem: Item {
                    scale: kbd.scale * 3 // make keys easily redable at 2.5 times the regular size

                    Key {
                        id: keyItem
                        key: root.key
                    }
                }

                background: null
            }
        }
    }

    Component {
        id: doodadShapeComponent
        ShapeDoodad {}
    }

    Component {
        id: doodadIndicatorComponent
        IndicatorDoodad {}
    }

    Component {
        id: doodadTextComponent
        TextDoodad {}
    }

    Component {
        id: doodadLogoComponent
        ShapeDoodad {
            KeyCapLabel {
                anchors.centerIn: parent
                color: disabledPalette.buttonText
                text: doodad.logo_name
                anchors.margins: 22 // arbitrary spacing to key outlines
            }
        }
    }

    Component {
        id: rowComponent

        Item {
            property QtObject row: null
            x: row.left
            y: row.top
            width: childrenRect.width+4
            height: childrenRect.height+4

            Component.onCompleted: {
                for (var i in row.keys) {
                    var key = row.keys[i]
                    keyComponent.createObject(this, { key: key });
                }
            }
        }
    }

    Component {
        id: sectionComponent

        Item {
            property QtObject section
            x: section.left
            y: section.top
            z: section.priority
            // Dimension definitions can be a bit wonky for sections but overally should not
            // cause problems even when they are (so long as we don't actually render the
            // sections themself)
            width: section.width
            height: section.height

            // Fix rotation to mod90, we cannot spin around as that'd put the text upside down ;)
            // Unclear if spinning around like that is in fact desired for anyting, if so I guess
            // we need to counter rotate text or something.
            rotation: section.angle != 0 ? section.angle % 90 : section.angle
            transformOrigin: Item.TopLeft

            Component.onCompleted: {
                for (var i in section.rows) {
                    var row = section.rows[i]
                    rowComponent.createObject(this, { row: row });
                }

                for (var i in section.doodads) {
                    var doodad = section.doodads[i]
                    createDoodad(doodad, kbd, { doodad: doodad })
                }

            }
        }
    }

    Component.onCompleted: {
        // Based on the geometry aspect we scale either width or height to scale so
        // the default size is fitting. NB: geom dimensions are mm!
        if (geometry.widthMM >= geometry.heightMM) {
            height = width * geometry.heightMM / geometry.widthMM
        } else {
            width = height * geometry.widthMM / geometry.heightMM
        }

        for (var i in geometry.sections) {
            var section = geometry.sections[i]
            sectionComponent.createObject(kbd, { section: section });
        }

        for (var i in geometry.doodads) {
            var doodad = geometry.doodads[i]
            createDoodad(doodad, kbd, { doodad: doodad })
        }

        visible = true
    }
}
