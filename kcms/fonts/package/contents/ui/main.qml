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
import org.kde.kcm 1.1 as KCM

KCM.SimpleKCM {
    id: root

    KCM.ConfigModule.quickHelp: i18n("Fonts")

    Kirigami.FormLayout {
        id: formLayout
        readonly property int maxImplicitWidth: Math.max(adjustAllFontsButton.implicitWidth, Math.max(antiAliasingComboBox.implicitWidth, Math.max(excludeField.implicitWidth, Math.max(subPixelCombo.implicitWidth, hintingCombo.implicitWidth))))

        QtControls.Button {
            id: adjustAllFontsButton
            Layout.preferredWidth: formLayout.maxImplicitWidth
            text: i18n("&Adjust All Fonts...")

            onClicked: kcm.adjustAllFonts();
        }

        FontWidget {
            id: generalFontWidget
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
            Layout.preferredWidth: formLayout.maxImplicitWidth
            Kirigami.FormData.label: i18nc("Used as a noun, and precedes a combobox full of options", "Anti-aliasing:")

            model: [i18n("Enabled"), i18n("Vendor Default"), i18n("Disabled")]

            currentIndex: kcm.fontAASettings.antiAliasing
            onCurrentIndexChanged: kcm.fontAASettings.antiAliasing = antiAliasingComboBox.currentIndex
        }

        QtControls.CheckBox {
            id: excludeCheckBox
            checked: kcm.fontAASettings.exclude
            onCheckedChanged: kcm.fontAASettings.exclude = checked;
            text: i18n("Exclude range from anti-aliasing")
            Layout.fillWidth: true
            enabled: antiAliasingComboBox.currentIndex == 0
        }

        RowLayout {
            id: excludeField
            Layout.preferredWidth: formLayout.maxImplicitWidth
            enabled: antiAliasingComboBox.currentIndex == 0
            QtControls.SpinBox {
                id: excludeFromSpinBox
                stepSize: 1
                onValueChanged: kcm.fontAASettings.excludeFrom = value
                textFromValue: function(value, locale) { return i18n("%1 pt", value)}
                enabled: excludeCheckBox.checked
            }

            QtControls.Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: i18n("to")
                enabled: excludeCheckBox.checked
            }

            QtControls.SpinBox {
                id: excludeToSpinBox
                stepSize: 1
                onValueChanged: kcm.fontAASettings.excludeTo = value
                textFromValue: function(value, locale) { return i18n("%1 pt", value)}
                enabled: excludeCheckBox.checked
            }
            Connections {
                target: kcm.fontAASettings
                onExcludeFromChanged: excludeFromSpinBox.value = kcm.fontAASettings.excludeFrom;
                onExcludeToChanged: excludeToSpinBox.value = kcm.fontAASettings.excludeTo;
            }
        }

        QtControls.ComboBox {
            id: subPixelCombo
            Layout.preferredWidth: formLayout.maxImplicitWidth
            Kirigami.FormData.label: i18nc("Used as a noun, and precedes a combobox full of options", "Sub-pixel rendering:")
            currentIndex: kcm.fontAASettings.subPixelCurrentIndex
            onCurrentIndexChanged: kcm.fontAASettings.subPixelCurrentIndex = currentIndex;
            model: kcm.fontAASettings.subPixelOptionsModel
            textRole: "display"
            enabled: antiAliasingComboBox.currentIndex == 0
            popup.height: popup.implicitHeight
            delegate: QtControls.ItemDelegate {
                id: subPixelDelegate
                onWidthChanged: {
                    subPixelCombo.popup.width = Math.max(subPixelCombo.popup.width, width)
                }
                contentItem: ColumnLayout {
                    id: subPixelLayout
                    Kirigami.Heading {
                        id: subPixelComboText
                        text: model.display
                        level: 5
                    }
                    Image {
                        id: subPixelComboImage
                        source: "image://preview/" + model.index + "_" + kcm.fontAASettings.hintingCurrentIndex + ".png"
                        // Setting sourceSize here is necessary as a workaround for QTBUG-38127
                        //
                        // With this bug, images requested from a QQuickImageProvider have an incorrect scale with devicePixelRatio != 1 when sourceSize is not set.
                        //
                        // TODO: Check if QTBUG-38127 is fixed and remove the next two lines.
                        sourceSize.width: 1
                        sourceSize.height: 1
                    }
                }
            }
        }

        QtControls.ComboBox {
            id: hintingCombo
            Layout.preferredWidth: formLayout.maxImplicitWidth
            Kirigami.FormData.label: i18nc("Used as a noun, and precedes a combobox full of options", "Hinting:")
            currentIndex: kcm.fontAASettings.hintingCurrentIndex
            onCurrentTextChanged: kcm.fontAASettings.hintingCurrentIndex = currentIndex;
            model: kcm.fontAASettings.hintingOptionsModel
            textRole: "display"
            enabled: antiAliasingComboBox.currentIndex == 0
            popup.height: popup.implicitHeight
            delegate: QtControls.ItemDelegate {
                id: hintingDelegate
                onWidthChanged: {
                    hintingCombo.popup.width = Math.max(hintingCombo.popup.width, width)
                }
                contentItem: ColumnLayout {
                    id: hintingLayout
                    Kirigami.Heading {
                        id: hintingComboText
                        text: model.display
                        level: 5
                    }
                    Image {
                        id: hintingComboImage
                        source: "image://preview/" + kcm.fontAASettings.subPixelCurrentIndex + "_" + model.index + ".png"
                        // Setting sourceSize here is necessary as a workaround for QTBUG-38127
                        //
                        // With this bug, images requested from a QQuickImageProvider have an incorrect scale with devicePixelRatio != 1 when sourceSize is not set.
                        //
                        // TODO: Check if QTBUG-38127 is fixed and remove the next two lines.
                        sourceSize.width: 1
                        sourceSize.height: 1
                    }
                }
            }
        }

        RowLayout {
            QtControls.CheckBox {
                id: dpiCheckBox
                checked: kcm.fontAASettings.dpi !== 0
                text: i18n("Force Fonts DPI:")
            }

            QtControls.SpinBox {
                id: dpiSpinBox
                stepSize: 24
                editable: true
                enabled: dpiCheckBox.checked
                value: enabled ? kcm.fontAASettings.dpi : 96

                Binding {
                    target: kcm
                    property: "fontAASettings.dpi"
                    value: dpiSpinBox.enabled ? dpiSpinBox.value : 0
                }
                to: 1000
                from: 1
            }
        }

        QtDialogs.FontDialog {
            id: fontDialog
            title: i18n("Choose a font")
            modality: Qt.WindowModal
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
    }
}

