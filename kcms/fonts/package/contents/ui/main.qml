/*
   Copyright (c) 2015 Antonis Tsiapaliokas <antonis.tsiapaliokas@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0 as QtControls
import QtQuick.Dialogs 1.2 as QtDialogs
//import QtQuick.Controls.Private 1.0
import org.kde.kirigami 2.3 as Kirigami
import org.kde.kcm 1.0

Kirigami.ScrollablePage {
    id: root

    ConfigModule.quickHelp: i18n("Fonts")


    Kirigami.FormLayout {
        id: formLayout

        QtControls.Button {
            id: adjustAllFontsButton
            text: i18n("Adjust All Fonts...")

            onClicked: {
                fontDialog.adjustAllFonts = true;
                fontDialog.open()
            }
        }

        Repeater {
            model: kcm.fontsModel

            delegate: RowLayout {
                id: row
                parent: formLayout

                Kirigami.FormData.label: model.display

                QtControls.TextField {
                    enabled: false
                    text: model.font.family + " " + model.font.pointSize
                    font: model.font
                    Layout.preferredHeight: fontButton.height
                }

                QtControls.Button {
                    id: fontButton
                    text: i18n("Choose...")
                    onClicked: {
                        fontDialog.adjustAllFonts = false;
                        fontDialog.currentIndex = index;
                        fontDialog.font = model.font;
                        fontDialog.open()
                    }
                }
            }
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        RowLayout {
            Kirigami.FormData.label: i18n("Use anti-aliasing:")

            QtControls.ComboBox {
                id: antiAliasingComboBox

                model: [i18n("Enabled"), i18n("System Settings"), i18n("Disabled")]

                currentIndex: kcm.fontAASettings.antiAliasing
                onCurrentIndexChanged: kcm.fontAASettings.antiAliasing = antiAliasingComboBox.currentIndex
            }

            QtControls.Button {
                text: i18n("Configure...")
                enabled: antiAliasingComboBox.currentIndex == 0

                onClicked: {
                    advancedDialog.open()
                }
            }
        }
        RowLayout {
            QtControls.CheckBox {
                id: dpiCheckBox
                checked: dpiSpinBox.value != 96
                text: i18n("Force Fonts DPI:")
                onCheckedChanged: {
                    if (!dpiCheckBox.checked) {
                        dpiSpinBox.enabled = false;
                        kcm.fontAASettings.dpi = 0;
                    } else {
                        dpiSpinBox.enabled = true;
                    }
                }
            }

            QtControls.SpinBox {
                id: dpiSpinBox
                stepSize: 24
                enabled: dpiCheckBox.checked
                value: kcm.fontAASettings.dpi
                onValueChanged: kcm.fontAASettings.dpi = dpiSpinBox.value
                to: 1000
                from: 96
            }
        }
    }


    QtDialogs.Dialog {
        id: advancedDialog
        title: i18n("Configure Anti-Alias Settings")
        standardButtons: QtDialogs.StandardButton.Ok | QtDialogs.StandardButton.Cancel

        onAccepted: {
           kcm.fontAASettings.subPixel = subPixelComboBox.currentText;
           kcm.fontAASettings.hinting = hintingComboBox.currentText;
           kcm.fontAASettings.excludeTo = excludeCheckBox.checked ? excludeToSpinBox.value : 0;
           kcm.fontAASettings.excludeFrom = excludeCheckBox.checked ? excludeFromSpinBox.value : 0;
        }

        onVisibleChanged: {
            //our dialog has open
            if (advancedDialog.visible) {
                subPixelComboBox.currentIndex = kcm.fontAASettings.subPixelCurrentIndex();
                hintingComboBox.currentIndex = kcm.fontAASettings.hintingCurrentIndex();
                excludeToSpinBox.value = kcm.fontAASettings.excludeTo;
                excludeFromSpinBox.value = kcm.fontAASettings.excludeFrom;
            }
        }

        ColumnLayout {
            RowLayout {
                QtControls.CheckBox {
                    id: excludeCheckBox
                    checked: excludeToSpinBox.value != 0 && excludeFromSpinBox.value != 0
                    text: i18n("Exclude Range")
                    Layout.fillWidth: true
                }

                QtControls.SpinBox {
                    id: excludeFromSpinBox
                    stepSize: 1
                    textFromValue: function(value, locale) { return i18n("%1 pt", value)}
                    enabled: excludeCheckBox.checked
                }

                QtControls.Label {
                    text: i18n("to")
                }

                QtControls.SpinBox {
                    id: excludeToSpinBox
                    stepSize: 1
                    textFromValue: function(value, locale) { return i18n("%1 pt", value)}
                    enabled: excludeCheckBox.checked
                }
            }

            RowLayout {
                QtControls.Label{
                    text: i18n("Sub-pixel rendering type:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }

                QtControls.ComboBox {
                    id: subPixelComboBox
                    currentIndex: kcm.fontAASettings.subPixelCurrentIndex()
                    model: kcm.fontAASettings.subPixelOptionsModel
                    textRole: "display"
                }
            }

            RowLayout {
                QtControls.Label {
                    text: i18n("Hinting style:")
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }

                QtControls.ComboBox {
                    id: hintingComboBox
                    currentIndex: kcm.fontAASettings.hintingCurrentIndex()
                    model: kcm.fontAASettings.hintingOptionsModel
                    textRole: "display"
                }
            }
        }
    }

    Connections {
        target: kcm.fontAASettings
        onSubPixelChanged: kcm.needsSave = true
        onHintingChanged: kcm.needsSave = true
        onExcludeToChanged: kcm.needsSave = true
        onExcludeFromChanged: kcm.needsSave = true
        //FIXME Those two signals are being emitted but the kcm.needsSave doesn't change.
        //onDpiChanged: kcm.needsSave = true
        //onAliasingChanged: kcm.needsSave = true;
    }

    QtDialogs.FontDialog {
        id: fontDialog
        title: "Choose a font"
        property int currentIndex
        property bool adjustAllFonts: false
        onAccepted: {
            if (adjustAllFonts) {
                kcm.adjustAllFonts(font)
            } else {
                kcm.updateFont(currentIndex, font)
            }
        }
    }

    QtDialogs.MessageDialog {
        id: messageDialog
        title: i18n("Settings Have Changed")
        text: i18n("Some changes such as anti-aliasing or DPI will only affect newly started applications.")
    }

    Connections {
        target: kcm
        onFontsHaveChanged: messageDialog.open()
    }
}

