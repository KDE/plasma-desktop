/*
 *    Copyright 2018 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

import QtQuick 2.1
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

import QtQuick.Controls.Styles 1.1

//my
import org.kde.plasma.components 3.0 as PlasmaComponents3

Item {
    id: root

    readonly property QtObject source: PlasmaCore.DataSource {
        id: keystateSource
        engine: "keystate"
        connectedSources: plasmoid.configuration.key
    }

    function translate(identifier) {
        switch(identifier) {
            case "Caps Lock": return i18n("Caps Lock")
            case "Num Lock": return i18n("Num Lock")
        }
        return identifier;
    }

    function icon(identifier) {
        switch(identifier) {
            case "Num Lock": return "input-num-on"
            case "Caps Lock": return "input-caps-on"
        }
        return null
    }

    readonly property bool lockedCount: {
        var ret = 0;
        for (var v in keystateSource.connectedSources) {
            var data = keystateSource.data[keystateSource.connectedSources[v]];
            ret += data && data.Locked
        }
        return ret
    }

    Plasmoid.icon: {
        for (var v in keystateSource.connectedSources) {
            var source = keystateSource.connectedSources[v]
            var data = keystateSource.data[source];
            if (data && data.Locked)
                return icon(source)
        }
        return "input-keyboard"
    }

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation
    Plasmoid.compactRepresentation: KeyboardLayoutButton {
        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 10
//	button.style: ButtonStyle {}
//	style: ButtonStyle {}
	contentItem: Item {
   id: root

    property string labelText: "text"

    readonly property bool usingFocusBackground: !parent.flat && buttonSvg.hasElement("hint-focus-highlighted-background") && parent.visualFocus && !(parent.pressed || parent.checked)
    
    PlasmaCore.ColorScope.inherit: true

//    columns: parent.display == T.Button.TextBesideIcon ? 2 : 1
//    columns: 1
    
//    rowSpacing: parent.spacing
//    columnSpacing: rowSpacing

PlasmaCore.IconItem {
        id: icon

        readonly property int defaultIconSize: root.parent.flat ? PlasmaCore.Units.iconSizes.smallMedium : PlasmaCore.Units.iconSizes.small

        Layout.alignment: Qt.AlignCenter

        Layout.fillWidth: true
        Layout.fillHeight: true

        Layout.minimumWidth: Math.min(parent.width, parent.height, implicitWidth)
        Layout.minimumHeight: Math.min(parent.width, parent.height, implicitHeight)

        Layout.maximumWidth: root.parent.icon.width > 0 ? root.parent.icon.width : Number.POSITIVE_INFINITY
        Layout.maximumHeight: root.parent.icon.height > 0 ? root.parent.icon.height : Number.POSITIVE_INFINITY

        implicitWidth: root.parent.icon.width > 0 ? root.parent.icon.width : defaultIconSize
        implicitHeight: root.parent.icon.height > 0 ? root.parent.icon.height : defaultIconSize

        visible: true
//        source: root.parent.icon ? (root.parent.icon.name || root.parent.icon.source) : ""
        source: "flag-green"
//        source: "face-smile"
        status: usingFocusBackground ? PlasmaCore.Svg.Selected : PlasmaCore.Svg.Normal
    }
    PlasmaComponents3.Label {
        id: label
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: true
        text: root.parent.text
        font: root.parent.font
        color: usingFocusBackground ? PlasmaCore.ColorScope.highlightedTextColor : PlasmaCore.ColorScope.textColor
//        horizontalAlignment: root.parent.display !== T.Button.TextUnderIcon && icon.visible ? Text.AlignLeft : Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    	}
//	contentItem: contentItem {}
//	contentItem.labelText: "asdf"
    }


    Plasmoid.fullRepresentation: ColumnLayout {
        PlasmaExtras.Heading {
            Layout.fillWidth: true
            level: 3
            wrapMode: Text.WordWrap
            text: root.Plasmoid.toolTipSubText
        }
        Item {
            Layout.fillHeight: true
            width: 5
        }
    }

    Plasmoid.status: lockedCount>0 ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus
    Plasmoid.toolTipSubText: {
        var ret = "";
        var found = false;
        for (var v in keystateSource.connectedSources) {
            var source = keystateSource.connectedSources[v]
            var data = keystateSource.data[source];
            if (data && data.Locked) {
                found = true
                ret+=i18n("%1: Locked\n", translate(source))
            }
        }
        return found ? ret.trim() : i18n("Unlocked")
    }
}
