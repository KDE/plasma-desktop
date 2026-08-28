// SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQml

import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KD
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    Kirigami.Form {
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                contentItem: QQC2.ComboBox {
                    id: defaultLanguage
                    Kirigami.FormData.label: i18nc("@label", "Default Language:")
                    model: kcm.settings.dictionaryModel
                    textRole: "display"
                    valueRole: "languageCode"

                    onActivated: {
                        kcm.settings.defaultLanguage = currentValue
                    }

                    Component.onCompleted: {
                        currentIndex = indexOfValue(kcm.settings.defaultLanguage)
                    }

                    Connections {
                        target: kcm.settings
                        function onDefaultLanguageChanged(): void {
                            defaultLanguage.currentIndex = defaultLanguage.indexOfValue(kcm.settings.defaultLanguage)
                        }
                    }
                }
                KCM.SettingHighlighter {
                    highlight: kcm.settings.defaultLanguage !== kcm.settings.defaultDefaultLanguage()
                }
            }
        }

        Kirigami.FormGroup {
            title: i18nc("@title", "Preferred Languages")

            Kirigami.FormEntry {
                contentItem: QQC2.Button {
                    text: i18nc("@action:button", "Configure Preferred Languages")
                    onClicked: {
                        let dialog = spellCheckingLanguageList.createObject(root) as Kirigami.Dialog;
                        dialog.open();
                    }
                    KCM.SettingHighlighter {
                        highlight: kcm.settings.preferredLanguages.length !== kcm.settings.defaultPreferredLanguages().length || !kcm.settings.preferredLanguages.every(value => kcm.settings.defaultPreferredLanguages().includes(value))
                    }

                    Component {
                        id: spellCheckingLanguageList
                        Kirigami.Dialog {
                            id: scroll
                            title: i18ndc("kirigami-addons6", "@title:window", "Spell checking languages")

                            width: Kirigami.Units.gridUnit * 24

                            standardButtons: QQC2.DialogButtonBox.Close
                            onRejected: scroll.close()

                            ListView {
                                clip: true
                                model: kcm.settings.dictionaryModel
                                delegate: KD.CheckSubtitleDelegate {
                                    id: delegate
                                    width: ListView.view.width

                                    required property var model
                                    required property bool isDefault

                                    text: model.display
                                    onClicked: delegate.model.checked = checked
                                    checked: delegate.model.checked
                                    Accessible.description: delegate.isDefault ? i18nd("kirigami-addons6", "Default Language") : ""
                                    icon.source: delegate.isDefault ? "favorite" : ""
                                }
                            }
                        }
                    }
                }
            }
        }

        Kirigami.FormGroup {
            title: i18nc("@title", "Options")

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    text: i18nc("@option:check", "Enable autodetection of language")
                    checked: kcm.settings.autodetectLanguage
                    onClicked: {
                        kcm.settings.autodetectLanguage = checked;
                    }
                }
                KCM.SettingHighlighter {
                    highlight: kcm.settings.autodetectLanguage !== kcm.settings.defaultAutodetectLanguage()
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    text: i18nc("@option:check", "Automatic spell checking enabled by default")
                    checked: kcm.settings.checkerEnabledByDefault
                    onCheckedChanged: kcm.settings.checkerEnabledByDefault = checked
                }
                KCM.SettingHighlighter {
                    highlight: kcm.settings.checkerEnabledByDefault !== kcm.settings.defaultCheckerEnabledByDefault()
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    text: i18nc("@option:check", "Skip all uppercase words")
                    checked: kcm.settings.skipUppercase
                    onCheckedChanged: kcm.settings.skipUppercase = checked
                }
                KCM.SettingHighlighter {
                    highlight: kcm.settings.skipUppercase !== kcm.settings.defaultSkipUppercase()
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    text: i18nc("@option:check", "Skip run-together words")
                    checked: kcm.settings.skipRunTogether
                    onCheckedChanged: kcm.settings.skipRunTogether = checked
                }
                KCM.SettingHighlighter {
                    highlight: kcm.settings.skipRunTogether !== kcm.settings.defaultSkipRunTogether()
                }
            }
        }

        Kirigami.FormGroup {
            title: i18nc("@title", "Ignored Words")

            Kirigami.FormEntry {
                contentItem: QQC2.Button {
                    text: i18nc("@action:button", "Configure Ignored Words")
                    onClicked: {
                        let dialog = ignoredWordsComponent.createObject(root) as Kirigami.Dialog;
                        dialog.open();
                    }
                }
                KCM.SettingHighlighter {
                    highlight: kcm.settings.currentIgnoreList.length !== kcm.settings.defaultIgnoreList().length || !kcm.settings.currentIgnoreList.every(value => kcm.settings.defaultIgnoreList().includes(value))

                }

                Component {
                    id: ignoredWordsComponent
                    Kirigami.Dialog {
                        id: dialog
                        title: i18nc("@title:dialog", "Ignored Words")

                        width: Kirigami.Units.gridUnit * 24

                        standardButtons: QQC2.DialogButtonBox.Close
                        onRejected: dialog.close()

                        footerLeadingComponent: Kirigami.ActionTextField {
                            id: newWord
                            placeholderText: "Dolphin"
                            Layout.fillWidth: true
                            rightActions: [
                                Kirigami.Action {
                                    text: i18nc("@action:button", "Add Word")
                                    icon.name: "list-add"
                                    onTriggered: {
                                        let words = kcm.settings.currentIgnoreList;
                                        words.push(newWord.text);
                                        kcm.settings.currentIgnoreList = words;
                                    }
                                }
                            ]
                        }

                        ListView {
                            model: kcm.settings.currentIgnoreList
                            delegate: QQC2.ItemDelegate {
                                id: delegate
                                width: ListView.view.width

                                required property string modelData

                                contentItem: Kirigami.TitleSubtitleWithActions  {
                                    title: delegate.modelData
                                    actions: [
                                        Kirigami.Action {
                                            text: i18nc("@action:button", "Delete")
                                            icon.name: "list-remove"
                                            onTriggered: {
                                                kcm.settings.currentIgnoreList = kcm.settings.currentIgnoreList.filter((value, _, _) => {
                                                    return value !== delegate.modelData;
                                                });
                                            }
                                        }
                                    ]
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
