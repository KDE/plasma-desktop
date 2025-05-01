/*
    SPDX-FileCopyrightText: 2025 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kquickcontrols as KQuickControls
import org.kde.kitemmodels as KItemModels
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kcmutils as KCM

import org.kde.plasma.private.kcm_keyboard as KCMKeyboard

KCM.SimpleKCM {
    id: root

    KItemModels.KSortFilterProxyModel {
        id: keyboardsProxy
        sourceModel: kcm?.keyboards ?? undefined
        sortRoleName: "description"
        sortOrder: Qt.AscendingOrder
    }

    /*
    KeyboardLayoutsModel {
        id: layoutsProxy
    }

    KItemModels.KSortFilterProxyModel {
        id: variantProxy
        sourceModel: kcm?.layouts ?? undefined

        filterRowCallback: function (row, parent) {
            // if (!layoutsView.currentItem) {
            //     console.log("no currentItem")
            //     //return false;
            // }
            //console.log("variantProxy.count: " + variantProxy.count)
            if (!layoutsView.currentText) {
                console.log("no currentText")
                return false;
            }

            const modelIndex = sourceModel.index(row, 0, parent);
            const currentName = sourceModel.data(modelIndex, KItemModels.KRoleNames.role("shortName"));
            const selectedName = layoutsView.currentText;
            console.log("*** layoutsView.currentText: " + layoutsView.currentValue + " == " + currentName)

            if (currentName !== selectedName) {
                return false;
            }

            return true; // FIXME

            if (searchField != null && searchField.text.length > 0) {
                const currentDescription = sourceModel.data(modelIndex);
                if (currentDescription.search(filterRegularExpression) < 0) {
                    return false;
                }
            }

            return true;
        }
    }
    */
    ColumnLayout {

        FormCard.FormHeader {
            title: i18n("Model & Layout")
        }

        FormCard.FormCard {
            FormCard.FormComboBoxDelegate {
                id: modelCombo

                text: i18n("Keyboard Model")
                //description: i18n("Size of the top panel (needs restart).")
                textRole: "description"
                valueRole: "name"
                model: keyboardsProxy
                //model: kcm.keyboards

                Component.onCompleted: {
                    selectCurrent()
                    console.log("Found " + keyboardsProxy.count + " keyboard models.");
                }

                Connections {
                    target: kcm?.keyboardSettings ?? undefined

                    function onKeyboardModelChanged(): void {
                        keyboardModelComboBox.selectCurrent()
                    }
                }

                function selectCurrent(): void {
                    currentIndex = indexOfValue(kcm.keyboardSettings.keyboardModel);
                    console.log("Set " + kcm.keyboardSettings.keyboardModel + " models.");
                }

                KCM.SettingStateBinding {
                    configObject: kcm.keyboardSettings
                    settingName: "keyboardModel"
                }
            }

            FormCard.FormSwitchDelegate {
                id: enableLayouts
                text: i18nc("@option:check", "Custom Layout")
                checked: kcm.keyboardSettings.configureLayouts
                onCheckedChanged: {
                    kcm.keyboardSettings.configureLayouts = checked
                }
                KCM.SettingStateBinding {
                    configObject: kcm.keyboardSettings
                    settingName: "configureLayouts"
                }
            }



            Repeater {
                id: list
                model: kcm.userLayoutModel

                delegate: FormCard.FormTextDelegate {
                    //leading: Kirigami.Icon {source: "user"}
                    text: layoutName
                    description: variantName
                }
            }

            FormCard.FormButtonDelegate {
                id: pickLayoutButton
                icon.name: "preferences-desktop-keyboard"
                text: i18n("Change Layout...")
                onClicked: _cLayoutDialog.incubateObject(root, {}, Qt.Asynchronous)
                enabled: kcm.keyboardSettings.configureLayouts
            }


            Timer {
                interval: 5000
                repeat: true
                running: true
                onTriggered: {
                    console.log("ULM: " +  kcm.userLayoutModel)
                    if (kcm.userLayoutModel.rowCount() == 0) {
                        console.log("No layout set")
                    } else {
                        console.log("userLayoutModel.count == " + kcm.userLayoutModel.rowCount())
                        var row = kcm.userLayoutModel[0];
                        if (row) {
                            console.log("Row:  " + row)
                            console.log("layout variant: " + row["layoutName"] + " " + row["variantName"] + " " + row["displayName"])
                        } else {
                            console.log("no layout set.")
                        }
                    }
                }
            }


            // FormCard.FormComboBoxDelegate {
            //     id: layoutsView
            //     text: i18n("Keyboard Layout")
            //     enabled: kcm.keyboardSettings.configureLayouts
            //     model: layoutsProxy
            //     textRole: "description"
            //     valueRole: "name"
            //     onCurrentTextChanged: {
            //         console.log("layoutsView currentText: " + currentText)
            //     }
            // }
            //
            // FormCard.FormComboBoxDelegate {
            //     id: variantCombo
            //     text: i18n("Layout Variant")
            //     enabled: kcm.keyboardSettings.configureLayouts
            //     textRole: "shortName"
            //     valueRole: "variantName" // FIXME: check if right!
            //     model: layoutsView.currentText ? variantProxy : []
            //     //model: variantProxy
            // }


        }

        FormCard.FormHeader {
            title: i18n("Key Repeat")
        }
        FormCard.FormCard {

            FormCard.FormSwitchDelegate {
                id: keyRepeatSwitch
                text: i18n("Customize Repeat")
                //description: i18n("")
                checked: kcm.miscSettings.keyboardRepeat === __internal.keyboardRepeatRepeat
                onToggled: kcm.miscSettings.keyboardRepeat = checked
                ? __internal.keyboardRepeatRepeat
                : __internal.keyboardRepeatNothing

                Accessible.name: i18nc("@label:checkbox accessible", "Key repeat")
                // onCheckedChanged: {
                //     if (checked != ShellSettings.Settings.animationsEnabled) {
                //         ShellSettings.Settings.animationsEnabled = checked;
                //     }
                // }
                KCM.SettingStateBinding {
                    configObject: kcm.miscSettings
                    settingName: "keyboardRepeat"
                }

            }


            /*
            QQC2.Label {
                // Force the label to use its own Kirigami.Theme object
                Kirigami.Theme.inherit: false
                text: i18nc("@label:textbox Key repeat delay", "Delay:")
                enabled: kcm.miscSettings.keyboardRepeat !== __internal.keyboardRepeatNothing
            }
            */

            FormCard.FormSpinBoxDelegate {
                id: repeatDelaySpinbox
                Layout.preferredWidth: Kirigami.Units.gridUnit * 8
                from: 10
                to: 5000
                stepSize: 50

                label: i18nc("@label:textbox Key repeat delay", "Delay:")
                value: kcm.miscSettings.repeatDelay
                onValueChanged: kcm.miscSettings.repeatDelay = value

                Accessible.name: i18nc("@label:spinbox accessible", "Key repeat delay %1", textFromValue(value))

                validator: IntValidator {
                    bottom: Math.min(repeatDelaySpinbox.from, repeatDelaySpinbox.to)
                    top: Math.max(repeatDelaySpinbox.from, repeatDelaySpinbox.to)
                }

                textFromValue: (value, locale) => {
                    return i18nc("@spinbox:value Repeat delay interval", "%1 ms", value)
                }

                valueFromText: (text, locale) => {
                    return Number.fromLocaleString(locale, text.replace(i18nc("@spinbox:value Repeat delay interval", "ms"), ""))
                }

                KCM.SettingStateBinding {
                    configObject: kcm.miscSettings
                    settingName: "repeatDelay"
                    extraEnabledConditions: kcm.miscSettings.keyboardRepeat !== __internal.keyboardRepeatNothing
                }
            }


            /*
            // QQC2.Label {
            //     // Force the label to use its own Kirigami.Theme object
            //     Kirigami.Theme.inherit: false
            //     text: i18nc("@label:textbox Key repeat rate", "Rate:")
            //     enabled: kcm.miscSettings.keyboardRepeat !== __internal.keyboardRepeatNothing
            // }
            */

            FormCard.FormSpinBoxDelegate {
                id: repeatRateSpinbox
                //Layout.preferredWidth: Kirigami.Units.gridUnit * 8
                from: 20
                to: 20000
                stepSize: 500
                label: i18nc("@label:textbox Key repeat rate", "Rate:")



                value: Math.round(kcm.miscSettings.repeatRate * 100)
                onValueChanged: kcm.miscSettings.repeatRate = value / 100

                Accessible.name: i18nc("@label:spinbox accessible", "Key repeat rate %1", textFromValue(value))

                validator: DoubleValidator {
                    bottom: Math.min(repeatRateSpinbox.from, repeatRateSpinbox.to)
                    top:  Math.max(repeatRateSpinbox.from, repeatRateSpinbox.to)
                }

                textFromValue: (value, locale) => {
                    return i18nc("@spinbox:value  Key repeat rate", "%1 repeats/s", value / 100)
                }

                valueFromText: (text, locale) => {
                    return Number.fromLocaleString(locale, text.replace(i18nc("@spinbox:value  Key repeat rate", "repeats/s"), "")) * 100
                }

                KCM.SettingStateBinding {
                    configObject: kcm.miscSettings
                    settingName: "repeatRate"
                    extraEnabledConditions: kcm.miscSettings.keyboardRepeat !== __internal.keyboardRepeatNothing
                }
            }

            FormCard.FormDelegateSeparator {
                above: testAreaText
                below: repeatRateSpinbox
            }

            FormCard.FormSectionText {
                id: testAreaText
                text: i18nc("@label:textbox", "Test area")
            }
            RowLayout {
                spacing: 0

                // Private.ContentItemLoader {
                //     Layout.rightMargin: visible ? root.leadingPadding : 0
                //     visible: root.leading
                //     implicitHeight: visible ? root.leading.implicitHeight : 0
                //     implicitWidth: visible ? root.leading.implicitWidth : 0
                //     contentItem: root.leading
                // }

                QQC2.TextField {
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                    Layout.fillWidth: true
                    Kirigami.FormData.label: i18nc("@label:textbox", "Test area:")
                    //Layout.preferredWidth: keyboardModelComboBox.width
                    placeholderText: i18nc("@info:placeholder", "Type here to test settings")
                }

                // Private.ContentItemLoader {
                //     Layout.leftMargin: visible ? root.trailingPadding : 0
                //     visible: root.trailing
                //     implicitHeight: visible ? root.trailing.implicitHeight : 0
                //     implicitWidth: visible ? root.trailing.implicitWidth : 0
                //     contentItem: root.trailing
                // }
            }

        }
    }

    Component {
        id: _cLayoutDialog

        LayoutDialog {
            visible: false
            isMobile: true

            Component.onCompleted: open()
            onAccepted: destroy()
            onRejected: destroy()
        }
    }

    QtObject {
        id: __internal

        readonly property string keyboardRepeatRepeat: "repeat"
        readonly property string keyboardRepeatNothing: "nothing"
        readonly property string keyboardRepeatAccent: "accent"

        // Default value for layoutLoopCount (e.g. we cannot use layout looping feature)
        readonly property int noLooping: -1

        // Minimum number of layouts that can be used with layout looping feature
        readonly property int minimumLoopingCount: 2

        // Minimum number of layouts currently available (X.org only allows to have 4 layouts)
        readonly property int minimumAvailableLoops: Math.min(kcm.maxGroupCount, list.count - 1)
    }
}
