/*
 *    SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
 *
 *    GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami.forms as Forms
import org.kde.kcmutils as KCMUtils

import org.kde.plasma.private.kcm_recentfiles as RecentFiles

KCMUtils.SimpleKCM {
    id: root

    Forms.Form {
        Forms.FormGroup {
            Forms.FormEntry {
                title: i18ndc("kcm_recentFiles", "@label", "Keep History:")
                contentItem: RowLayout {
                    QQC2.SpinBox {
                        from: 0
                        to: 999
                        value: root.KCMUtils.ConfigModule.keepHistoryFor
                        onValueModified: root.KCMUtils.ConfigModule.keepHistoryFor = value

                        readonly property regexp numberExtractionRegExp: /\D*?(-?\d*\.?\d*)\D*$/
                        textFromValue: (value) => {
                            return i18ndcp("kcm_recentFiles", "@option The number of months to remember recent file history", "For %1 month", "For %1 months", value)
                        }
                        valueFromText: (text, locale) => {
                            return Number.fromLocaleString(locale, numberExtractionRegExp.exec(text)[1])
                        }
                    }
                    QQC2.ComboBox {
                        displayText: i18ndc("kcm_recentFiles", "@label", "Clear history")
                        textRole: "name"
                        valueRole: "value"
                        model: [
                            {
                                name: i18ndc("kcm_recentFiles", "@option", "Forget the last hour"),
                                value: 0,
                                amount: 1,
                                type: "h"
                            },
                            {
                                name: i18ndc("kcm_recentFiles", "@option", "Forget the last two hours"),
                                value: 1,
                                amount: 2,
                                type: "h"
                            },
                            {
                                name: i18ndc("kcm_recentFiles", "@option", "Forget the last day"),
                                value: 2,
                                amount: 1,
                                type: "d"
                            },
                            {
                                name: i18ndc("kcm_recentFiles", "@option", "Forget everything"),
                                value: 3,
                                amount: 0,
                                type: "everything"
                            }
                        ]

                        onActivated: console.warn(currentValue, model[currentValue].amount, model[currentValue].type)
                    }
                }
            }
            QQC2.ButtonGroup {
                id: applicationHistoryGroup
            }
            Forms.FormEntry {
                title: i18ndc("kcm_recentFiles", "@label", "Remember opened documents:")
                contentItem: QQC2.RadioButton {
                    text: i18nc("@option:radio", "For all applications")
                    checked: root.KCMUtils.ConfigModule.whatToRemember === 0
                    onToggled: if (checked) {
                        root.KCMUtils.ConfigModule.whatToRemember = 0
                    }

                    QQC2.ButtonGroup.group: applicationHistoryGroup
                }
            }
            Forms.FormEntry {
                contentItem: QQC2.RadioButton {
                    text: i18nc("@option:radio", "Do not remember")
                    checked: root.KCMUtils.ConfigModule.whatToRemember === 2
                    onToggled: if (checked) {
                        root.KCMUtils.ConfigModule.whatToRemember = 2
                    }

                    QQC2.ButtonGroup.group: applicationHistoryGroup
                }
            }
            Forms.FormEntry {
                contentItem: QQC2.RadioButton {
                    text: i18nc("@option:radio", "Only for specific applications")
                    checked: root.KCMUtils.ConfigModule.whatToRemember === 1
                    onToggled: if (checked) {
                        root.KCMUtils.ConfigModule.whatToRemember = 1
                    }

                    QQC2.ButtonGroup.group: applicationHistoryGroup
                }
            }
            Forms.FormEntry {
                fullWidth: true
                contentItem: ExcludedApplicationView {
                    Layout.fillWidth: true
                    enabled: root.KCMUtils.ConfigModule.whatToRemember === 1
                    model: root.KCMUtils.ConfigModule.excludedApplicationsModel
                    pageWidth: root.width
                }
            }
            Forms.FormEntry {
                fullWidth: true
                contentItem: QQC2.CheckBox {
                    enabled: root.KCMUtils.ConfigModule.whatToRemember === 1
                    text: i18nc("@option:radio", "Exclude applications not on the list")
                    checked: root.KCMUtils.ConfigModule.blockByDefault
                    onToggled: root.KCMUtils.ConfigModule.blockByDefault = !root.KCMUtils.ConfigModule.blockByDefault
                }
            }
        }
    }
}
