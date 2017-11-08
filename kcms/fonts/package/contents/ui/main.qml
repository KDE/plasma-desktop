/*
   Copyright (c) 2015 Antonis Tsiapaliokas <antonis.tsiapaliokas@kde.org>
   Copyright (c) 2017 Marco Martin <mart@kde.org>

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

        FontWidget {
            label: i18n("General:")
            category: "generalFont"
            font: kcm.generalFont
        }
        FontWidget {
            label: i18n("Fixed width:")
            category: "fixedWidthFont"
            font: kcm.fixedWidthFont
        }
        FontWidget {
            label: i18n("Small:")
            category: "smallFont"
            font: kcm.smallFont
        }
        FontWidget {
            label: i18n("Toolbar:")
            category: "toolbarFont"
            font: kcm.toolbarFont
        }
        FontWidget {
            label: i18n("Menu:")
            category: "menuFont"
            font: kcm.menuFont
        }
        FontWidget {
            label: i18n("Window title:")
            category: "windowTitleFont"
            font: kcm.windowTitleFont
        }
        
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        QtControls.ComboBox {
            id: antiAliasingComboBox
            Kirigami.FormData.label: i18n("Use anti-aliasing:")

            model: [i18n("Enabled"), i18n("System Settings"), i18n("Disabled")]

            currentIndex: kcm.fontAASettings.antiAliasing
            onCurrentIndexChanged: kcm.fontAASettings.antiAliasing = antiAliasingComboBox.currentIndex
        }

        QtControls.CheckBox {
            id: excludeCheckBox
            checked: excludeToSpinBox.value != 0 && excludeFromSpinBox.value != 0
            text: i18n("Exclude range from anti-aliasing")
            Layout.fillWidth: true
            visible: antiAliasingComboBox.currentIndex == 0
        }

        RowLayout {
            visible: antiAliasingComboBox.currentIndex == 0
            QtControls.SpinBox {
                id: excludeFromSpinBox
                stepSize: 1
                value: kcm.fontAASettings.excludeFrom
                onValueChanged: kcm.fontAASettings.excludeFrom = value
                textFromValue: function(value, locale) { return i18n("%1 pt", value)}
                enabled: excludeCheckBox.checked
            }

            QtControls.Label {
                text: i18n("to")
            }

            QtControls.SpinBox {
                id: excludeToSpinBox
                stepSize: 1
                value: kcm.fontAASettings.excludeTo
                onValueChanged: kcm.fontAASettings.excludeTo = value
                textFromValue: function(value, locale) { return i18n("%1 pt", value)}
                enabled: excludeCheckBox.checked
            }
        }

        QtControls.ComboBox {
            Kirigami.FormData.label: i18n("Sub-pixel rendering type:")
            currentIndex: kcm.fontAASettings.subPixelCurrentIndex()
            model: kcm.fontAASettings.subPixelOptionsModel
            textRole: "display"
            visible: antiAliasingComboBox.currentIndex == 0
        }

        QtControls.ComboBox {
            Kirigami.FormData.label: i18n("Hinting style:")
            currentIndex: kcm.fontAASettings.hintingCurrentIndex()
            model: kcm.fontAASettings.hintingOptionsModel
            textRole: "display"
            visible: antiAliasingComboBox.currentIndex == 0
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

    QtDialogs.FontDialog {
        id: fontDialog
        title: "Choose a font"
        property string currentCategory
        property bool adjustAllFonts: false
        onAccepted: {
            if (adjustAllFonts) {
                kcm.adjustAllFonts(font);
            } else {
                kcm[currentCategory] = font;
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

