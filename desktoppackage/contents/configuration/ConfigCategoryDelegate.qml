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
import org.kde.plasma.core 2.0 as PlasmaCore

MouseArea {
    id: delegate

//BEGIN properties
    y: units.smallSpacing *2
    width: parent.width
    height: delegateContents.height + units.smallSpacing * 4
    hoverEnabled: true
    property bool current: model.source == main.sourceFile
    property string name: model.name
//END properties

//BEGIN functions
    function openCategory() {
        if (typeof(categories.currentItem) !== "undefined") {
            main.invertAnimations = (categories.currentItem.y > delegate.y);
            categories.currentItem = delegate;
        }
        main.sourceFile = model.source
        main.title = model.name
    }
//END functions

//BEGIN connections
    onClicked: {
        print("model source: " + model.source + " " + main.sourceFile);
        if (root.configurationHasChanged()) {
            messageDialog.delegate = delegate
            messageDialog.open();
            return;
        }
        if (delegate.current) {
            return;
        } else {
            openCategory();
        }
    }
    onCurrentChanged: {
        if (current) {
            categories.currentItem = delegate;
        }
    }
//END connections

//BEGIN UI components
    Rectangle {
        anchors.fill: parent
        color: syspal.highlight
        opacity: {
            if (categories.currentItem == delegate) {
                return 1
            } else if (delegate.containsMouse) {
                return 0.3 // there's no "hover" color in SystemPalette
            } else {
                return 0
            }
        }
        Behavior on opacity {
            NumberAnimation {
                duration: units.longDuration
            }
        }
    }

    Column {
        id: delegateContents
        spacing: units.smallSpacing
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            right: parent.right
        }
        PlasmaCore.IconItem {
            anchors.horizontalCenter: parent.horizontalCenter
            width: theme.IconSizeHuge
            height: width
            source: model.icon
        }
        QtControls.Label {
            anchors {
                left: parent.left
                right: parent.right
            }
            text: model.name
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            color: current ? syspal.highlightedText : syspal.text
            Behavior on color {
                ColorAnimation {
                    duration: units.longDuration
                    easing.type: "InOutQuad"
                }
            }
        }
    }
//END UI components
}
