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
import org.kde.kcmutils as KCMUtils
import org.kde.plasma.workspace.timezoneselector as TimeZone

import org.kde.plasma.private.kcm_clock as DateTime

KCMUtils.SimpleKCM {
    id: root

    header: MessageViewer {
        id: messageViewer
        messages: root.KCMUtils.ConfigModule.messages
    }

    Kirigami.Form {
        Kirigami.FormGroup {
            title: i18ndc("kcmclock", "@title", "Date and Time")

            Kirigami.FormEntry {
                title: i18ndc("kcmclock", "@title", "Current date:")
                contentItem: RowLayout {
                    QQC2.Label {
                        Layout.fillWidth: true
                        Layout.minimumHeight: dateButton.implicitHeight
                        text: root.KCMUtils.ConfigModule.dateString
                        verticalAlignment: Text.AlignVCenter
                    }
                    QQC2.Button {
                        id: dateButton
                        visible: !root.KCMUtils.ConfigModule.ntpEnabled
                        text: i18ndc("kcmclock", "@action:button as in set the current date on the machine", "Set date")
                        onClicked: {
                            let dialog = Qt.createComponent("org.kde.kirigamiaddons.dateandtime", "DatePopup").createObject(QQC2.Overlay.overlay, {
                                width: 300,
                                height: 300,
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
            Kirigami.FormEntry {
                title: i18ndc("kcmclock", "@title", "Current time:")
                contentItem: RowLayout {
                    QQC2.Label {
                        Layout.fillWidth: true
                        Layout.minimumHeight: dateButton.implicitHeight
                        text: root.KCMUtils.ConfigModule.timeString
                        verticalAlignment: Text.AlignVCenter
                    }
                    QQC2.Button {
                        id: timeButton
                        visible: !root.KCMUtils.ConfigModule.ntpEnabled
                        text: i18ndc("kcmclock", "@action:button as in set the current time on the machine", "Set time")
                        onClicked: {
                            let dialog = Qt.createComponent("org.kde.kirigamiaddons.dateandtime", "TimePopup").createObject(QQC2.Overlay.overlay, {
                                width: 200,
                                height: 300,
                                value: root.KCMUtils.ConfigModule.dateTime
                            }) as DateAndTime.TimePopup;
                            dialog.onAccepted.connect(() => {
                                root.KCMUtils.ConfigModule.setTime(dialog.value);
                            });
                            dialog.open();
                        }
                    }
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    text: i18ndc("kcmclock", "@option:check enable this mouse device", "Set date and time automatically")
                    enabled: root.KCMUtils.ConfigModule.ntpAvailable
                    checked: root.KCMUtils.ConfigModule.ntpEnabled

                    onToggled: root.KCMUtils.ConfigModule.ntpEnabled = !root.KCMUtils.ConfigModule.ntpEnabled
                }
            }
        }
        Kirigami.FormGroup {
            title: i18ndc("kcmclock", "@title", "Time Zone")

            Kirigami.FormEntry {
                contentItem: TimeZone.TimezoneSelector {
                    id: selector
                    implicitWidth: 600
                    implicitHeight: width * 3 / 4

                    Component.onCompleted: selectedTimeZone = root.KCMUtils.ConfigModule.timeZone

                    Connections {
                        target: root.KCMUtils.ConfigModule
                        function onTimeZoneChanged(): void {
                            selector.selectedTimeZone = root.KCMUtils.ConfigModule.timeZone
                        }
                    }

                    onSelectedTimeZoneChanged: {
                        root.KCMUtils.ConfigModule.timeZone = selectedTimeZone
                    }
                }
            }
        }
    }
}
