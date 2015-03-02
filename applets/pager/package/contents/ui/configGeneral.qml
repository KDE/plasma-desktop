    /*
 *  Copyright 2013 David Edmundson <davidedmundson@kde.org>
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
import QtQuick.Layouts 1.0 as Layouts


//FIXME this causes a crash in Oxygen style
//QtControls.GroupBox {
Item {

    width: childrenRect.width
    height: childrenRect.height

//FIXME enable when we're back to being a group box
//     flat: true
//     title: i18n("Appearance")

    property int cfg_displayedText
    property alias cfg_showWindowIcons: showWindowIcons.checked
    property int cfg_currentDesktopSelected

    onCfg_displayedTextChanged: {
        switch (cfg_displayedText) {
        case 0:
            displayedTextGroup.current = desktopNumberRadio;
            break;
        case 1:
            displayedTextGroup.current = desktopNameRadio;
            break;
        default:
        case 2:
            displayedTextGroup.current = noTextRadio;
            break;
        }
    }

    onCfg_currentDesktopSelectedChanged: {
        switch (cfg_currentDesktopSelected) {
        case 0:
            currentDesktopSelectedGroup.current = doesNothingRadio;
            break;
        case 1:
            currentDesktopSelectedGroup.current = showsDesktopRadio;
            break;
        default:
        case 2:
            currentDesktopSelectedGroup.current = showsDashboardRadio;
            break;
        }
    }

    Component.onCompleted: {
        cfg_currentDesktopSelectedChanged();
        cfg_displayedTextChanged();
    }

    QtControls.ExclusiveGroup {
        id: displayedTextGroup
    }
    QtControls.ExclusiveGroup {
        id: currentDesktopSelectedGroup
    }

    Layouts.GridLayout {
        columns: 2
        QtControls.Label {
            text: i18n("Display:")
            Layouts.Layout.alignment: Qt.AlignVCenter|Qt.AlignRight
        }
        QtControls.RadioButton {
            id: desktopNumberRadio
            exclusiveGroup: displayedTextGroup
            text: i18n("Desktop number")
            onCheckedChanged: if (checked) cfg_displayedText = 0;
        }
        Item {
            width: 2
            height: 2
            Layouts.Layout.rowSpan: 2
        }
        QtControls.RadioButton {
            id: desktopNameRadio
            exclusiveGroup: displayedTextGroup
            text: i18n("Desktop name")
            onCheckedChanged: if (checked) cfg_displayedText = 1;
        }
        QtControls.RadioButton {
            id: noTextRadio
            exclusiveGroup: displayedTextGroup
            text: i18n("No text")
            onCheckedChanged: if (checked) cfg_displayedText = 2;
        }

        Item {
            width: 2
            height: 2
        } //spacer

        QtControls.CheckBox {
            id: showWindowIcons
            text: i18n("Icons")
        }

        QtControls.Label {
            text: i18n("Selecting current desktop:")
            Layouts.Layout.alignment: Qt.AlignVCenter|Qt.AlignRight
        }
        QtControls.RadioButton {
            id: doesNothingRadio
            exclusiveGroup: currentDesktopSelectedGroup
            text: i18n("Does nothing")
            onCheckedChanged: if (checked) cfg_currentDesktopSelected = 0;
        }
        Item {
            width: 2
            height: 2
            Layouts.Layout.rowSpan: 2
        }
        QtControls.RadioButton {
            id: showsDesktopRadio
            exclusiveGroup: currentDesktopSelectedGroup
            text: i18n("Shows desktop")
            onCheckedChanged: if (checked) cfg_currentDesktopSelected = 1;
        }
        QtControls.RadioButton {
            id: showsDashboardRadio
            exclusiveGroup: currentDesktopSelectedGroup
            text: i18n("Shows the dashboard")
            onCheckedChanged: if (checked) cfg_currentDesktopSelected = 2;
        }
    }

}
