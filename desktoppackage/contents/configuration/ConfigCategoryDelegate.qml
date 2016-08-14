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
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Window 2.2

import org.kde.kquickcontrolsaddons 2.0

MouseArea {
    id: delegate

//BEGIN properties
    implicitWidth: delegateContents.implicitWidth + 4 * units.smallSpacing
    implicitHeight: delegateContents.height + units.smallSpacing * 4
    Layout.fillWidth: true
    hoverEnabled: true

    property bool current: (model.kcm && main.currentItem.kcm && model.kcm == main.currentItem.kcm) || (model.source == main.sourceFile)
//END properties

//BEGIN functions
    function openCategory() {
        if (current) {
            return;
        }
        if (typeof(categories.currentItem) !== "undefined") {
            main.invertAnimations = (categories.currentItem.y > delegate.y);
            categories.currentItem = delegate;
        }
        if (model.source) {
            main.sourceFile = model.source;
        } else if (model.kcm) {
            main.sourceFile = "";
            main.sourceFile = Qt.resolvedUrl("ConfigurationKcmPage.qml");
            main.currentItem.kcm = model.kcm;
        } else {
            main.sourceFile = "";
        }
        main.title = model.name
    }
//END functions

//BEGIN connections
    onClicked: {
        categoriesScroll.forceActiveFocus()

        if (current) {
            return;
        }

        //print("model source: " + model.source + " " + main.sourceFile);
        if (applyButton.enabled) {
            messageDialog.delegate = delegate;
            messageDialog.open();
            return;
        }

        openCategory();
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
        opacity: { // try to match Breeze style hover handling
            var active = categoriesScroll.activeFocus && Window.active
            if (current) {
                if (active) {
                    return 1
                } else if (delegate.containsMouse) {
                    return 0.6
                } else {
                    return 0.3
                }
            } else if (delegate.containsMouse) {
                if (active) {
                    return 0.3
                } else {
                    return 0.1
                }
            }
            return 0
        }
        Behavior on opacity {
            NumberAnimation {
                duration: units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    ColumnLayout {
        id: delegateContents
        spacing: units.smallSpacing
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter

        QIconItem {
            id: iconItem
            Layout.alignment: Qt.AlignHCenter
            width: units.iconSizes.medium
            height: width
            icon: model.icon
            state: current ? QIconItem.SelectedState : QIconItem.DefaultState
        }

        QtControls.Label {
            id: nameLabel
            Layout.fillWidth: true
            text: model.name
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            color: current && categoriesScroll.activeFocus ? syspal.highlightedText : syspal.text
            Behavior on color {
                ColorAnimation {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
//END UI components
}
