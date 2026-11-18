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

    Kirigami.Form {
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@title:group prefix for checkbox group", "General:")
                contentItem: QQC2.CheckBox {
                    id: showWindowOutlines
                    text: i18nc("@option:check", "Show window outlines")
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: showWindowIcons
                    leftPadding: mirrored ? 0 : showWindowOutlines.indicator.width + showWindowOutlines.spacing
                    rightPadding: mirrored ? showWindowOutlines.indicator.width + showWindowOutlines.spacing : 0
                    enabled: showWindowOutlines.checked
                    text: i18nc("@option:check", "Show application icons")
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: showOnlyCurrentScreen
                    text: i18nc("@option:check", "Show only current screen")
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: wrapPage
                    text: i18nc("@option:check", "Navigation wraps around")
                }
            }
        }

        Kirigami.FormGroup {
            visible: root.isActivityPager
            Kirigami.FormEntry {
                title: i18nc("@title:listbox", "Layout:")
                contentItem: QQC2.ComboBox {
                    id: pagerLayout

                    model: [i18nc("@item:inlistbox The pager layout", "Default"), i18nc("@item:inlistbox The pager layout", "Horizontal"), i18nc("@item:inlistbox The pager layout", "Vertical")]
                }
            }
        }

        Kirigami.FormGroup {
            QQC2.ButtonGroup { id: displayedTextGroup }

            Kirigami.FormEntry {
                title: i18nc("@title:group prefix for radiobutton group", "Text display:")
                contentItem: QQC2.RadioButton {
                    id: noTextRadio
                    QQC2.ButtonGroup.group: displayedTextGroup
                    text: i18nc("@option:radio text display", "No text")
                    checked: root.cfg_displayedText === 2
                    onToggled: if (checked) root.cfg_displayedText = 2;
                }
            }

            Kirigami.FormEntry {
                QQC2.ButtonGroup { id: currentDesktopSelectedGroup }
                contentItem: QQC2.RadioButton {
                    id: desktopNumberRadio
                    QQC2.ButtonGroup.group: displayedTextGroup
                    text: root.isActivityPager ? i18nc("@option:radio text display", "Activity number") : i18nc("@option:radio text display", "Desktop number")
                    checked: root.cfg_displayedText === 0
                    onToggled: if (checked) root.cfg_displayedText = 0;
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    id: desktopNameRadio
                    QQC2.ButtonGroup.group: displayedTextGroup
                    text: root.isActivityPager ? i18nc("@option:radio text display", "Activity name") : i18nc("@option:radio text display", "Desktop name")
                    checked: root.cfg_displayedText === 1
                    onToggled: if (checked) root.cfg_displayedText = 1;
                }
            }

        }

        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: root.isActivityPager
                    ? i18nc("@label Start of the sentence 'Selecting current activity does nothing/shows the desktop'", "Selecting current Activity:")
                    : i18nc("@label Start of the sentence 'Selecting current virtual desktop does nothing/shows the desktop'", "Selecting current virtual desktop:")
                contentItem: QQC2.RadioButton {
                    id: doesNothingRadio
                    QQC2.ButtonGroup.group: currentDesktopSelectedGroup
                    text: i18nc("option:check completes the sentence 'Selecting current activity/virtual desktop does nothing'", "Does nothing")
                    checked: root.cfg_currentDesktopSelected === 0
                    onToggled: if (checked) root.cfg_currentDesktopSelected = 0;
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    id: showsDesktopRadio
                    QQC2.ButtonGroup.group: currentDesktopSelectedGroup
                    text: i18nc("option:check completes the sentence 'Selecting current activity/virtual desktop shows the desktop'", "Shows the desktop")
                    checked: root.cfg_currentDesktopSelected === 1
                    onToggled: if (checked) root.cfg_currentDesktopSelected = 1;
                }
            }
        }
    }
}
