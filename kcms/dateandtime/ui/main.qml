/*
    SPDX-FileCopyrightText: 2024 Niccolò Venerandi <niccolo@venerandi.com>
    SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.dateandtime as DateAndTime
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kcmutils as KCMUtils
import org.kde.plasma.workspace.timezoneselector as TimeZone

import org.kde.plasma.private.kcm_clock as DateTime

pragma ComponentBehavior: Bound

KCMUtils.SimpleKCM {
    id: root

    header: MessageViewer {
        messages: root.KCMUtils.ConfigModule.messages
    }

    Kirigami.Form {
        Kirigami.FormGroup {
            title: i18ndc("kcm_clock", "@title", "Date and Time")

            Kirigami.FormEntry {
                contentItem: QQC2.Label{
                    Layout.fillWidth: true
                    text: root.KCMUtils.ConfigModule.dateTimeString
                    color: Kirigami.Theme.textColor
                    wrapMode: Text.Wrap
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    text: i18ndc("kcm_clock", "@option", "Set date and time automatically")
                    enabled: root.KCMUtils.ConfigModule.ntpAvailable
                    checked: root.KCMUtils.ConfigModule.ntpEnabled

                    onToggled: root.KCMUtils.ConfigModule.ntpEnabled = !root.KCMUtils.ConfigModule.ntpEnabled
                }
            }
            Kirigami.FormEntry {
                visible: !root.KCMUtils.ConfigModule.ntpEnabled
                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.Button {
                        id: timeButton
                        Layout.fillWidth: true
                        text: i18ndc("kcm_clock", "@action:button as in set the current time on the machine", "Set Time")
                        onClicked: {
                            let dialog = Qt.createComponent("org.kde.kirigamiaddons.dateandtime", "TimePopup").createObject(QQC2.Overlay.overlay, {
                                width: Kirigami.Units.gridUnit * 12,
                                height: Kirigami.Units.gridUnit * 18,
                                value: root.KCMUtils.ConfigModule.dateTime
                            }) as DateAndTime.TimePopup;
                            dialog.onAccepted.connect(() => {
                                root.KCMUtils.ConfigModule.setTime(dialog.value);
                            });
                            dialog.open();
                        }
                    }
                    QQC2.Button {
                        id: dateButton
                        Layout.fillWidth: true
                        text: i18ndc("kcm_clock", "@action:button as in set the current date on the machine", "Set Date")
                        onClicked: {
                            let dialog = Qt.createComponent("org.kde.kirigamiaddons.dateandtime", "DatePopup").createObject(QQC2.Overlay.overlay, {
                                width: Kirigami.Units.gridUnit * 18,
                                height: Kirigami.Units.gridUnit * 18,
                                value: root.KCMUtils.ConfigModule.dateTime
                            }) as DateAndTime.DatePopup;
                            dialog.onAccepted.connect(() => {
                                root.KCMUtils.ConfigModule.setDate(dialog.value);
                            });
                            dialog.open();
                        }
                    }
                }
            }
        }
        Kirigami.FormGroup {
            title: i18ndc("kcm_clock", "@title", "Time Zone")

            Kirigami.FormEntry {
                contentItem: QQC2.Label{
                    Layout.fillWidth: true
                    text: root.KCMUtils.ConfigModule.timeZoneString
                    color: Kirigami.Theme.textColor
                    wrapMode: Text.Wrap
                }
            }
            Kirigami.FormEntry {
                contentItem: TimeZone.TimezoneSelector {
                    id: timeZoneSelector
                    Layout.fillWidth: true
                    implicitWidth: Kirigami.Units.gridUnit * 50
                    implicitHeight: Math.round(width * 3 / 4)

                    selectedTimeZone: root.KCMUtils.ConfigModule.timeZone

                    onSelectedTimeZoneChanged: {
                        root.KCMUtils.ConfigModule.timeZone = selectedTimeZone
                    }

                    Connections {
                        target: root.KCMUtils.ConfigModule
                        function onTimeZoneChanged(): void {
                            timeZoneSelector.selectedTimeZone = root.KCMUtils.ConfigModule.timeZone
                        }
                    }
                }
            }
        }
    }
}
