/*
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: root
    width: 200
    height: 300
    clip: true

    Rectangle { 
        color: "white"
        opacity: 0.0
        anchors.fill: parent
    }
    PlasmaExtras.Title {
        id: title
        anchors { top: parent.top; left: parent.left; right: parent.right; }
        text: i18n("Test Applet")
    }
    PlasmaExtras.Paragraph {
        id: para
        anchors { top: title.bottom; left: parent.left; right: parent.right; }
        text: {
            var o = "x: " + root.x;
            o += "<br />y: " + root.y;
            o += "<br />w: " + root.width;
            o += "<br />h: " + root.height;
            //o += "<br /><br />s: " + plasmoid.availableScreenRect(screen[0]);
            return o;
        }
    }

    PlasmaComponents.ButtonColumn {
        visible: root.width + root.height > 500
        anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter; }
        spacing: 12
        //anchors.fill: parent
        PlasmaComponents.Button {
            text: hint
            iconSource: "view-refresh"
            property string hint: "NoBackground"
            checkable: true
            checked: plasmoid.backgroundHints == hint
            onClicked: saveHints(hint);
        }
        PlasmaComponents.Button {
            text: hint
            iconSource: "view-refresh"
            property string hint: "TranslucentBackground"
            checkable: true
            checked: plasmoid.backgroundHints == hint
            onClicked: saveHints(hint);
        }
        PlasmaComponents.Button {
            text: hint
            iconSource: "view-refresh"
            property string hint: "NormalBackground"
            checkable: true
            checked: plasmoid.backgroundHints == hint
            onClicked: saveHints(hint);
        }
        PlasmaComponents.Button {
            text: hint
            iconSource: "view-refresh"
            property string hint: "DefaultBackground"
            checkable: true
            checked: plasmoid.backgroundHints == hint
            onClicked: saveHints(hint);
        }
    }

    function saveHints(hint) {
        print("writing to config: " + hint);
        plasmoid.writeConfig("bgHints", hint);
    }

    function configChanged()
    {
//         //serviceUrl = plasmoid.readConfig("serviceUrl");
//         var u = plasmoid.readConfig("userName");
//         var s = plasmoid.readConfig("serviceUrl");
//         print(" @@@@@@@@@@@@@@@@@@ configChanged()" + u + " s: " + s);
// 
//         if (u != "") {
//             userName = u;
//         }
//         if (s != "") {
//             serviceUrl = s;
//             imageSourc    
        var bgh = "DefaultBackground";
        var _bgh = plasmoid.readConfig("bgHints");
        print("-- " + _bgh);
        if (_bgh != "") {
            bgh = _bgh;
        } else {
            //bgh = ;
            print("Default");
        }
//         if (typeof(bgh) == "undefined" || bgh == undefined || bgh == "") {
//         }
        print(" ----------? Read Background Hints from Config: " + bgh + " " + (typeof(_bgh)));

        //plasmoid.backgroundHints = "DefaultBackground";
        plasmoid.backgroundHints = "NormalBackground";
    }

    Timer {
        repeat: false
        //running: true
        interval: 100
        //onTriggered: configChanged()
    }

    Component.onCompleted: {
        //plasmoid.backgroundHints = "NoBackground"
        //plasmoid.addEventListener('ConfigChanged', configChanged);
        //plasmoid.configurationRequired = true
        configChanged()
    }
}