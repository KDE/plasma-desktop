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
                contentItem: ColumnLayout {
                    spacing: Kirigami.Units.smallSpacing
                    RowLayout {
                        spacing: Kirigami.Units.largeSpacing
                        QQC2.Label{
                            Layout.fillWidth: true
                            text: root.KCMUtils.ConfigModule.timeString
                            color: Kirigami.Theme.textColor
                            font {
                                pointSize: Kirigami.Theme.defaultFont.pointSize * 2
                                weight: Font.DemiBold
                            }
                        }
                        QQC2.Button {
                            id: timeButton
                            visible: !root.KCMUtils.ConfigModule.ntpEnabled
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
                    }
                    RowLayout {
                        spacing: Kirigami.Units.largeSpacing
                        QQC2.Label{
                            Layout.fillWidth: true
                            text: root.KCMUtils.ConfigModule.dateString
                            color: Kirigami.Theme.textColor
                            font {
                                pointSize: Kirigami.Theme.defaultFont.pointSize * 2
                                weight: Font.DemiBold
                            }
                        }
                        QQC2.Button {
                            id: dateButton
                            visible: !root.KCMUtils.ConfigModule.ntpEnabled
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
            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    text: i18ndc("kcm_clock", "@option", "Set date and time automatically")
                    enabled: root.KCMUtils.ConfigModule.ntpAvailable
                    checked: root.KCMUtils.ConfigModule.ntpEnabled

                    onToggled: root.KCMUtils.ConfigModule.ntpEnabled = !root.KCMUtils.ConfigModule.ntpEnabled
                }
            }
        }
        Kirigami.FormGroup {
            title: i18ndc("kcm_clock", "@title", "Time Zone")

            Kirigami.FormEntry {
                contentItem: TimeZone.TimezoneSelector {
                    id: timeZoneSelector
                    implicitWidth: Kirigami.Units.gridUnit * 50
                    implicitHeight: Math.round(width * 3 / 4)

                    selectedTimeZone: root.KCMUtils.ConfigModule.timeZone

                    onSelectedTimeZoneChanged: {
                        root.KCMUtils.ConfigModule.timeZone = selectedTimeZone
                    }

                    Connections {
                        target: root.KCMUtils.ConfigModule
                        function onTimeZoneChanged(): void {
                            selector.selectedTimeZone = root.KCMUtils.ConfigModule.timeZone
                        }
                    }
                }
            }
            Kirigami.FormEntry {
                clickEnabled: false
                contentItem: TimeZone.TimeZoneComboBox {
                    selector: timeZoneSelector

                    onSelected: timeZone => root.KCMUtils.ConfigModule.timeZone = timeZone
                }
            }
        }
    }
}
