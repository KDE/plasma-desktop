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
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kcm 1.1 as KCM

KCM.SimpleKCM {
    id: root

    KCM.ConfigModule.quickHelp: i18n("This module lets you configure the system fonts.")

    ColumnLayout {

        Kirigami.InlineMessage {
            id: antiAliasingMessage
            Layout.fillWidth: true
            showCloseButton: true
            text: i18n("Some changes such as anti-aliasing or DPI will only affect newly started applications.")

            Connections {
                target: kcm.fontAASettings
                onAliasingChangeApplied: antiAliasingMessage.visible = true
            }
        }

        Kirigami.FormLayout {
            id: formLayout
            readonly property int maxImplicitWidth: Math.max(adjustAllFontsButton.implicitWidth, excludeField.implicitWidth, subPixelCombo.implicitWidth, hintingCombo.implicitWidth)

            QtControls.Button {
                id: adjustAllFontsButton
                Layout.preferredWidth: formLayout.maxImplicitWidth
                icon.name: "font-select-symbolic"
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

            QtControls.CheckBox {
                id: antiAliasingCheckBox
                checked: kcm.fontAASettings.antiAliasing
                onCheckedChanged: kcm.fontAASettings.antiAliasing = checked
                Kirigami.FormData.label: i18n("Anti-Aliasing:")
                text: i18n("Enable")
                Layout.fillWidth: true
            }

            QtControls.CheckBox {
                id: excludeCheckBox
                checked: kcm.fontAASettings.exclude
                onCheckedChanged: kcm.fontAASettings.exclude = checked;
                text: i18n("Exclude range from anti-aliasing")
                Layout.fillWidth: true
                enabled: antiAliasingCheckBox.checked
            }

            RowLayout {
                id: excludeField
                Layout.preferredWidth: formLayout.maxImplicitWidth
                enabled: antiAliasingCheckBox.checked
                QtControls.SpinBox {
                    id: excludeFromSpinBox
                    stepSize: 1
                    onValueChanged: kcm.fontAASettings.excludeFrom = value
                    textFromValue: function(value, locale) { return i18n("%1 pt", value)}
                    valueFromText: function(text, locale) { return parseInt(text) }
                    editable: true
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
                    valueFromText: function(text, locale) { return parseInt(text) }
                    editable: true
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
                enabled: antiAliasingCheckBox.checked
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
                enabled: antiAliasingCheckBox.checked
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
                    text: i18n("Force font DPI:")
                    onClicked: kcm.fontAASettings.dpi = (checked ? dpiSpinBox.value : 0)
                }

                QtControls.SpinBox {
                    id: dpiSpinBox
                    stepSize: 24
                    editable: true
                    enabled: dpiCheckBox.checked
                    value: kcm.fontAASettings.dpi !== 0 ? kcm.fontAASettings.dpi : 96
                    onValueModified: kcm.fontAASettings.dpi = value
                    // to: need to divide to stepSize
                    to: 1008
                    // lowest vaue here can be == stepSize, that is because 0 means off
                    from: 24
                }
            }

            QtDialogs.FontDialog {
                id: fontDialog
                title: i18n("Select Font")
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
}
