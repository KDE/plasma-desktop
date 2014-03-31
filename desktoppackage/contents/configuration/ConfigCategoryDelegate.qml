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

    anchors {
        left: parent.left
        right: parent.right
    }
//BEGIN properties
    width: childrenRect.width
    height: childrenRect.height + 4
    property bool current: model.source == main.sourceFile
    property string name: model.name
//END properties

//BEGIN connections
    onClicked: {
        print("model source: " + model.source + " " + main.sourceFile);
        if (delegate.current) {
            return
        } else {
            if (typeof(categoriesView.currentItem) !== "undefined") {
                categoriesView.currentItem = delegate;
            }
            main.sourceFile = model.source
            main.title = model.name
            root.restoreConfig()
        }
    }
    onCurrentChanged: {
        if (current) {
            categoriesView.currentItem = delegate;
        }
    }
//END connections

//BEGIN UI components
    Column {
        spacing: 4
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            right: parent.right
            topMargin: _m
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
        }
    }
//END UI components
}
