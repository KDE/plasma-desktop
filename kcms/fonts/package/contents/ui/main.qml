/*
   Copyright (c) 2015 Antonis Tsiapaliokas <antonis.tsiapaliokas@kde.org>
   Copyright (c) 2017 Marco Martin <mart@kde.org>
   Copyright (c) 2019 Benjamin Port <benjamin.port@enioka.com>

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

// For KCMShell.open()
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kcm 1.1 as KCM

KCM.SimpleKCM {
    id: root

    KCM.ConfigModule.quickHelp: i18n("This module lets you configure the system fonts.")

    Kirigami.Action {
        id: kscreenAction
        visible: KCMShell.authorize("kcm_kscreen.desktop").length > 0
        text: i18n("Change Display Scaling...")
        iconName: "preferences-desktop-display"
        onTriggered: KCMShell.open("kcm_kscreen.desktop")
    }

    ColumnLayout {

        Kirigami.InlineMessage {
            id: antiAliasingMessage
            Layout.fillWidth: true
            showCloseButton: true
            text: i18n("Some changes such as anti-aliasing or DPI will only affect newly started applications.")

            Connections {
                target: kcm
                onAliasingChangeApplied: antiAliasingMessage.visible = true
            }
        }

        Kirigami.InlineMessage {
            id: hugeFontsMessage
            Layout.fillWidth: true
            showCloseButton: true
            text: i18n("Very large fonts may produce odd-looking results. Consider adjusting the global screen scale instead of using a very large font size.")

            Connections {
                target: kcm
                onFontsHaveChanged: {
                    hugeFontsMessage.visible = generalFontWidget.font.pointSize > 14
                    || fixedWidthFontWidget.font.pointSize > 14
                    || smallFontWidget.font.pointSize > 14
                    || toolbarFontWidget.font.pointSize > 14
                    || menuFontWidget.font.pointSize > 14
                }
            }

            actions: [ kscreenAction ]
        }

        Kirigami.InlineMessage {
            id: dpiTwiddledMessage
            Layout.fillWidth: true
            showCloseButton: true
            text: i18n("The recommended way to scale the user interface is using the global screen scaling feature.")
            actions: [ kscreenAction ]
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
                enabled: !kcm.fontsSettings.isImmutable("font")
                        || !kcm.fontsSettings.isImmutable("fixed")
                        || !kcm.fontsSettings.isImmutable("smallestReadableFont")
                        || !kcm.fontsSettings.isImmutable("toolBarFont")
                        || !kcm.fontsSettings.isImmutable("menuFont")
                        || !kcm.fontsSettings.isImmutable("activeFont")
            }

            FontWidget {
                id: generalFontWidget
                label: i18n("General:")
                category: "font"
                font: kcm.fontsSettings.font
                enabled: !kcm.fontsSettings.isImmutable("font")
            }
            FontWidget {
                id: fixedWidthFontWidget
                label: i18n("Fixed width:")
                category: "fixed"
                font: kcm.fontsSettings.fixed
                enabled: !kcm.fontsSettings.isImmutable("fixed")
            }
            FontWidget {
                id: smallFontWidget
                label: i18n("Small:")
                category: "smallestReadableFont"
                font: kcm.fontsSettings.smallestReadableFont
                enabled: !kcm.fontsSettings.isImmutable("smallestReadableFont")
            }
            FontWidget {
                id: toolbarFontWidget
                label: i18n("Toolbar:")
                category: "toolBarFont"
                font: kcm.fontsSettings.toolBarFont
                enabled: !kcm.fontsSettings.isImmutable("toolBarFont")
            }
            FontWidget {
                id: menuFontWidget
                label: i18n("Menu:")
                category: "menuFont"
                font: kcm.fontsSettings.menuFont
                enabled: !kcm.fontsSettings.isImmutable("menuFont")
            }
            FontWidget {
                label: i18n("Window title:")
                category: "activeFont"
                font: kcm.fontsSettings.activeFont
                enabled: !kcm.fontsSettings.isImmutable("activeFont")
            }

            Kirigami.Separator {
                Kirigami.FormData.isSection: true
            }

            QtControls.CheckBox {
                id: antiAliasingCheckBox
                checked: kcm.fontsAASettings.antiAliasing
                onCheckedChanged: kcm.fontsAASettings.antiAliasing = checked
                Kirigami.FormData.label: i18n("Anti-Aliasing:")
                text: i18n("Enable")
                Layout.fillWidth: true
            }

            QtControls.CheckBox {
                id: excludeCheckBox
                checked: kcm.fontsAASettings.exclude
                onCheckedChanged: kcm.fontsAASettings.exclude = checked;
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
                    onValueChanged: kcm.fontsAASettings.excludeFrom = value
                    textFromValue: function(value, locale) { return i18n("%1 pt", value)}
                    valueFromText: function(text, locale) { return parseInt(text) }
                    editable: true
                    enabled: excludeCheckBox.checked
                    value: kcm.fontsAASettings.excludeFrom
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
                    onValueChanged: kcm.fontsAASettings.excludeTo = value
                    textFromValue: function(value, locale) { return i18n("%1 pt", value)}
                    valueFromText: function(text, locale) { return parseInt(text) }
                    editable: true
                    enabled: excludeCheckBox.checked
                    value: kcm.fontsAASettings.excludeTo
                }
                Connections {
                    target: kcm.fontsAASettings
                    onExcludeFromChanged: excludeFromSpinBox.value = kcm.fontsAASettings.excludeFrom;
                    onExcludeToChanged: excludeToSpinBox.value = kcm.fontsAASettings.excludeTo;
                }
            }

            QtControls.ComboBox {
                id: subPixelCombo
                Layout.preferredWidth: formLayout.maxImplicitWidth
                Kirigami.FormData.label: i18nc("Used as a noun, and precedes a combobox full of options", "Sub-pixel rendering:")
                currentIndex: kcm.subPixelCurrentIndex
                onCurrentIndexChanged: kcm.subPixelCurrentIndex = currentIndex;
                model: kcm.subPixelOptionsModel
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
                            source: "image://preview/" + model.index + "_" + kcm.hintingCurrentIndex + ".png"
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
                currentIndex: kcm.hintingCurrentIndex
                onCurrentTextChanged: kcm.hintingCurrentIndex = currentIndex;
                model: kcm.hintingOptionsModel
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
                            source: "image://preview/" + kcm.subPixelCurrentIndex + "_" + model.index + ".png"
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
                    checked: kcm.fontsAASettings.dpi !== 0
                    text: i18n("Force font DPI:")
                    onClicked: {
                        kcm.fontsAASettings.dpi = checked ? dpiSpinBox.value : 0
                        dpiTwiddledMessage.visible = checked
                    }
                }

                QtControls.SpinBox {
                    id: dpiSpinBox
                    editable: true
                    enabled: dpiCheckBox.checked
                    value: kcm.fontsAASettings.dpi !== 0 ? kcm.fontsAASettings.dpi : 96
                    onValueModified: kcm.fontsAASettings.dpi = value
                    to: 999
                    from: 1
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
                        kcm.adjustAllFonts()
                    } else {
                        kcm.adjustFont(font, currentCategory)
                    }
                }
            }
        }
    }
}
