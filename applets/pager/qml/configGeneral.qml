    /*
 *  SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>
 *  SPDX-FileCopyrightText: 2016 Eike Hein <hein@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root
    readonly property bool isActivityPager: Plasmoid.pluginName === "org.kde.plasma.activitypager"

    property int cfg_displayedText
    property alias cfg_showWindowOutlines: showWindowOutlines.checked
    property alias cfg_showWindowIcons: showWindowIcons.checked
    property int cfg_currentDesktopSelected
    property alias cfg_pagerLayout: pagerLayout.currentIndex
    property alias cfg_showOnlyCurrentScreen: showOnlyCurrentScreen.checked
    property alias cfg_wrapPage: wrapPage.checked

    Kirigami.FormLayout {
        QQC2.ButtonGroup {
            id: displayedTextGroup
        }

        QQC2.ButtonGroup {
            id: currentDesktopSelectedGroup
        }

        QQC2.CheckBox {
            id: showWindowOutlines

            Kirigami.FormData.label: i18nc("@title:group prefix for checkbox group", "General:") // qmllint disable unqualified

            text: i18nc("@option:check", "Show window outlines") // qmllint disable unqualified
        }

        QQC2.CheckBox {
            id: showWindowIcons
            text: i18nc("@option:check", "Show application icons on window outlines") // qmllint disable unqualified
            enabled: showWindowOutlines.checked
        }

        QQC2.CheckBox {
            id: showOnlyCurrentScreen
            text: i18nc("@option:check", "Show only current screen") // qmllint disable unqualified
        }

        QQC2.CheckBox {
            id: wrapPage
            text: i18nc("@option:check", "Navigation wraps around") // qmllint disable unqualified
        }


        Item {
            Kirigami.FormData.isSection: true
        }


        QQC2.ComboBox {
            id: pagerLayout

            Kirigami.FormData.label: i18nc("@title:listbox", "Layout:") // qmllint disable unqualified

            model: [i18nc("@item:inlistbox The pager layout", "Default"), i18nc("@item:inlistbox The pager layout", "Horizontal"), i18nc("@item:inlistbox The pager layout", "Vertical")] // qmllint disable unqualified
            visible: root.isActivityPager
        }


        Item {
            Kirigami.FormData.isSection: true
            visible: root.isActivityPager
        }


        QQC2.RadioButton {
            id: noTextRadio

            Kirigami.FormData.label: i18nc("@title:group prefix for radiobutton group", "Text display:") // qmllint disable unqualified

            QQC2.ButtonGroup.group: displayedTextGroup
            text: i18nc("@option:radio text display", "No text") // qmllint disable unqualified
            checked: root.cfg_displayedText === 2
            onToggled: if (checked) root.cfg_displayedText = 2;
        }

        QQC2.RadioButton {
            id: desktopNumberRadio
            QQC2.ButtonGroup.group: displayedTextGroup
            text: isActivityPager ? i18nc("@option:radio text display", "Activity number") : i18nc("@option:radio text display", "Desktop number") // qmllint disable unqualified
            checked: root.cfg_displayedText === 0
            onToggled: if (checked) root.cfg_displayedText = 0;
        }

        QQC2.RadioButton {
            id: desktopNameRadio
            QQC2.ButtonGroup.group: displayedTextGroup
            text: isActivityPager ? i18nc("@option:radio text display", "Activity name") : i18nc("@option:radio text display", "Desktop name") // qmllint disable unqualified
            checked: root.cfg_displayedText === 1
            onToggled: if (checked) root.cfg_displayedText = 1;
        }


        Item {
            Kirigami.FormData.isSection: true
        }


        QQC2.RadioButton {
            id: doesNothingRadio

            Kirigami.FormData.label: root.isActivityPager
                ? i18nc("@label Start of the sentence 'Selecting current activity does nothing/shows the desktop'", "Selecting current Activity:") // qmllint disable unqualified
                : i18nc("@label Start of the sentence 'Selecting current virtual desktop does nothing/shows the desktop'", "Selecting current virtual desktop:") // qmllint disable unqualified

            QQC2.ButtonGroup.group: currentDesktopSelectedGroup
            text: i18nc("option:check completes the sentence 'Selecting current activity/virtual desktop does nothing'", "Does nothing") // qmllint disable unqualified
            checked: root.cfg_currentDesktopSelected === 0
            onToggled: if (checked) root.cfg_currentDesktopSelected = 0;
        }

        QQC2.RadioButton {
            id: showsDesktopRadio
            QQC2.ButtonGroup.group: currentDesktopSelectedGroup
            text: i18nc("option:check completes the sentence 'Selecting current activity/virtual desktop shows the desktop'", "Shows the desktop") // qmllint disable unqualified
            checked: root.cfg_currentDesktopSelected === 1
            onToggled: if (checked) root.cfg_currentDesktopSelected = 1;
        }
    }
}
