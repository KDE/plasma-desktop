/*
 *    SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
 *
 *    GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.ki18n
import org.kde.kirigami.forms as Forms
import org.kde.kcmutils as KCMUtils

KCMUtils.SimpleKCM {
    id: root

    Forms.Form {
        Forms.FormGroup {
            Forms.FormEntry {
                title: KI18n.i18ndc("kcm_recentFiles", "@label", "Keep History:")
                subtitle: KI18n.i18ndc("kcm_recentFiles", "@label", "This setting only applies to applications that support recent file tracking via KActivityManager")
                contentItem: RowLayout {
                    QQC2.SpinBox {
                        from: 0
                        to: 61
                        wrap: true
                        value: kcm.settings.keepHistoryFor
                        onValueModified: kcm.settings.keepHistoryFor = value

                        readonly property regexp numberExtractionRegExp: /\D*?(-?\d*\.?\d*)\D*$/
                        textFromValue: (value) => {
                            if (value === 0) {
                                return KI18n.i18ndc("kcm_recentFiles", "@option Do not remember file history", "Never");
                            } else if (value === 61) {
                                return KI18n.i18ndc("kcm_recentFiles", "@option unlimited number of months", "Forever");
                            }
                            return KI18n.i18ndcp("kcm_recentFiles", "@option The number of months to remember recent file history", "For %1 month", "For %1 months", value)
                        }
                        valueFromText: (text, locale) => {
                            return Number.fromLocaleString(locale, numberExtractionRegExp.exec(text)[1])
                        }

                        KCMUtils.SettingStateBinding {
                            configObject: kcm.settings
                            settingName: "keepHistoryFor"
                        }
                    }
                    QQC2.ComboBox {
                        displayText: KI18n.i18ndc("kcm_recentFiles", "@label", "Clear history")
                        textRole: "name"
                        valueRole: "value"
                        model: [
                            {
                                name: KI18n.i18ndc("kcm_recentFiles", "@option", "Forget the last hour"),
                                value: 0,
                                amount: 1,
                                type: "h"
                            },
                            {
                                name: KI18n.i18ndc("kcm_recentFiles", "@option", "Forget the last two hours"),
                                value: 1,
                                amount: 2,
                                type: "h"
                            },
                            {
                                name: KI18n.i18ndc("kcm_recentFiles", "@option", "Forget the last day"),
                                value: 2,
                                amount: 1,
                                type: "d"
                            },
                            {
                                name: KI18n.i18ndc("kcm_recentFiles", "@option", "Forget everything"),
                                value: 3,
                                amount: 0,
                                type: "everything"
                            }
                        ]

                        onActivated: kcm.forget(model[currentValue].amount, model[currentValue].type)
                    }
                }
            }
        }
    }
}
