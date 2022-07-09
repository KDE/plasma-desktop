    /*
 *  SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>
 *  SPDX-FileCopyrightText: 2016 Eike Hein <hein@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.5
import QtQuick.Controls 2.5 as QtControls

import org.kde.kirigami 2.5 as Kirigami


Kirigami.FormLayout {

    anchors.left: parent.left
    anchors.right: parent.right

    property bool isActivityPager: (plasmoid.pluginName === "org.kde.plasma.activitypager")

    property int cfg_displayedText
    property alias cfg_showWindowIcons: showWindowIcons.checked
    property int cfg_currentDesktopSelected
    property alias cfg_pagerLayout: pagerLayout.currentIndex
    property alias cfg_showOnlyCurrentScreen: showOnlyCurrentScreen.checked
    property alias cfg_wrapPage: wrapPage.checked

    QtControls.ButtonGroup {
        id: displayedTextGroup
    }
    QtControls.ButtonGroup {
        id: currentDesktopSelectedGroup
    }


    QtControls.CheckBox {
        id: showWindowIcons

        Kirigami.FormData.label: i18n("General:")

        text: i18n("Show application icons on window outlines")
    }

    QtControls.CheckBox {
        id: showOnlyCurrentScreen
        text: i18n("Show only current screen")
    }

    QtControls.CheckBox {
        id: wrapPage
        text: i18n("Navigation wraps around")
    }


    Item {
        Kirigami.FormData.isSection: true
    }


    QtControls.ComboBox {
        id: pagerLayout

        Kirigami.FormData.label: i18n("Layout:")

        model: [i18nc("The pager layout", "Default"), i18n("Horizontal"), i18n("Vertical")]
        visible: isActivityPager
    }


    Item {
        Kirigami.FormData.isSection: true
        visible: isActivityPager
    }


    QtControls.RadioButton {
        id: noTextRadio

        Kirigami.FormData.label: i18n("Text display:")

        QtControls.ButtonGroup.group: displayedTextGroup
        text: i18n("No text")
        checked: cfg_displayedText === 2
        onToggled: if (checked) cfg_displayedText = 2;
    }
    QtControls.RadioButton {
        id: desktopNumberRadio
        QtControls.ButtonGroup.group: displayedTextGroup
        text: isActivityPager ? i18n("Activity number") : i18n("Desktop number")
        checked: cfg_displayedText === 0
        onToggled: if (checked) cfg_displayedText = 0;
    }
    QtControls.RadioButton {
        id: desktopNameRadio
        QtControls.ButtonGroup.group: displayedTextGroup
        text: isActivityPager ? i18n("Activity name") : i18n("Desktop name")
        checked: cfg_displayedText === 1
        onToggled: if (checked) cfg_displayedText = 1;
    }


    Item {
        Kirigami.FormData.isSection: true
    }


    QtControls.RadioButton {
        id: doesNothingRadio

        Kirigami.FormData.label: isActivityPager ? i18n("Selecting current Activity:") : i18n("Selecting current virtual desktop:")

        QtControls.ButtonGroup.group: currentDesktopSelectedGroup
        text: i18n("Does nothing")
        checked: cfg_currentDesktopSelected === 0
        onToggled: if (checked) cfg_currentDesktopSelected = 0;
    }
    QtControls.RadioButton {
        id: showsDesktopRadio
        QtControls.ButtonGroup.group: currentDesktopSelectedGroup
        text: i18n("Shows the desktop")
        checked: cfg_currentDesktopSelected === 1
        onToggled: if (checked) cfg_currentDesktopSelected = 1;
    }
}
