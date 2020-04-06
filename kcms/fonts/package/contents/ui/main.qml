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
import org.kde.kcm 1.3 as KCM

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

                KCM.SettingStateBinding {
                    configObject: kcm.fontsSettings
                    settingName: "font"
                }
            }
            FontWidget {
                id: fixedWidthFontWidget
                label: i18n("Fixed width:")
                category: "fixed"
                font: kcm.fontsSettings.fixed

                KCM.SettingStateBinding {
                    configObject: kcm.fontsSettings
                    settingName: "fixed"
                }
            }
            FontWidget {
                id: smallFontWidget
                label: i18n("Small:")
                category: "smallestReadableFont"
                font: kcm.fontsSettings.smallestReadableFont

                KCM.SettingStateBinding {
                    configObject: kcm.fontsSettings
                    settingName: "smallestReadableFont"
                }
            }
            FontWidget {
                id: toolbarFontWidget
                label: i18n("Toolbar:")
                category: "toolBarFont"
                font: kcm.fontsSettings.toolBarFont

                KCM.SettingStateBinding {
                    configObject: kcm.fontsSettings
                    settingName: "toolBarFont"
                }
            }
            FontWidget {
                id: menuFontWidget
                label: i18n("Menu:")
                category: "menuFont"
                font: kcm.fontsSettings.menuFont

                KCM.SettingStateBinding {
                    configObject: kcm.fontsSettings
                    settingName: "menuFont"
                }
            }
            FontWidget {
                label: i18n("Window title:")
                category: "activeFont"
                font: kcm.fontsSettings.activeFont

                KCM.SettingStateBinding {
                    configObject: kcm.fontsSettings
                    settingName: "activeFont"
                }
            }

            Kirigami.Separator {
                Kirigami.FormData.isSection: true
            }

            RowLayout {
                Kirigami.FormData.label: i18n("Anti-Aliasing:")
                QtControls.CheckBox {
                    id: antiAliasingCheckBox
                    checked: kcm.fontsAASettings.antiAliasing
                    onCheckedChanged: kcm.fontsAASettings.antiAliasing = checked
                    text: i18n("Enable")
                    Layout.fillWidth: true
                }
                ContextualHelpButton {
                    toolTipText: xi18nc("@info:tooltip Anti-Aliasing", "Pixels on displays are generally aligned in a grid. Therefore shapes of fonts that do not align with this grid will look blocky and wrong unless <emphasis>anti-aliasing</emphasis> techniques are used to reduce this effect. You generally want to keep this option enabled unless it causes problems.")
                }

                KCM.SettingStateBinding {
                    configObject: kcm.fontsAASettings
                    settingName: "antiAliasing"
                    extraEnabledConditions: !kcm.fontsAASettings.isAaImmutable
                }
            }

            QtControls.CheckBox {
                id: excludeCheckBox
                checked: kcm.fontsAASettings.exclude
                onCheckedChanged: kcm.fontsAASettings.exclude = checked;
                text: i18n("Exclude range from anti-aliasing")
                Layout.fillWidth: true

                KCM.SettingStateBinding {
                    configObject: kcm.fontsAASettings
                    settingName: "exclude"
                    extraEnabledConditions: !kcm.fontsAASettings.isAaImmutable && antiAliasingCheckBox.checked
                }
            }

            RowLayout {
                id: excludeField
                Layout.preferredWidth: formLayout.maxImplicitWidth
                enabled: antiAliasingCheckBox.enabled && antiAliasingCheckBox.checked

                QtControls.SpinBox {
                    id: excludeFromSpinBox
                    stepSize: 1
                    onValueChanged: kcm.fontsAASettings.excludeFrom = value
                    textFromValue: function(value, locale) { return i18n("%1 pt", value)}
                    valueFromText: function(text, locale) { return parseInt(text) }
                    editable: true
                    value: kcm.fontsAASettings.excludeFrom

                    KCM.SettingStateBinding {
                        configObject: kcm.fontsAASettings
                        settingName: "excludeFrom"
                        extraEnabledConditions: excludeCheckBox.checked
                    }
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
                    value: kcm.fontsAASettings.excludeTo

                    KCM.SettingStateBinding {
                        configObject: kcm.fontsAASettings
                        settingName: "excludeTo"
                        extraEnabledConditions: excludeCheckBox.checked
                    }
                }
                Connections {
                    target: kcm.fontsAASettings
                    onExcludeFromChanged: excludeFromSpinBox.value = kcm.fontsAASettings.excludeFrom;
                    onExcludeToChanged: excludeToSpinBox.value = kcm.fontsAASettings.excludeTo;
                }
            }

            RowLayout {
                Kirigami.FormData.label: i18nc("Used as a noun, and precedes a combobox full of options", "Sub-pixel rendering:")
                QtControls.ComboBox {
                    id: subPixelCombo
                    Layout.preferredWidth: formLayout.maxImplicitWidth
                    currentIndex: kcm.subPixelCurrentIndex
                    onCurrentIndexChanged: kcm.subPixelCurrentIndex = currentIndex;
                    model: kcm.subPixelOptionsModel
                    textRole: "display"
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

                    KCM.SettingStateBinding {
                        configObject: kcm.fontsAASettings
                        settingName: "subPixel"
                        extraEnabledConditions: antiAliasingCheckBox.checked && !kcm.fontsAASettings.isAaImmutable
                    }
                }
                ContextualHelpButton {
                    toolTipText: xi18nc("@info:tooltip Sub-pixel rendering", "<para>On TFT or LCD screens every single pixel is actually composed of three or four smaller monochrome lights. These <emphasis>sub-pixels</emphasis> can be changed independently to further improve the quality of displayed fonts.</para> <para>The rendering quality is only improved if the selection matches the manner in which the sub-pixels of your display are aligned. Most displays have a linear ordering of <emphasis>RGB</emphasis> sub-pixels, some have <emphasis>BGR</emphasis> and some exotic orderings are not supported by this feature.</para>This does not work with CRT monitors.")
                }
            }

            RowLayout {
                Kirigami.FormData.label: i18nc("Used as a noun, and precedes a combobox full of options", "Hinting:")
                QtControls.ComboBox {
                    id: hintingCombo
                    Layout.preferredWidth: formLayout.maxImplicitWidth
                    currentIndex: kcm.hintingCurrentIndex
                    onCurrentTextChanged: kcm.hintingCurrentIndex = currentIndex;
                    model: kcm.hintingOptionsModel
                    textRole: "display"
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
                    KCM.SettingStateBinding {
                        configObject: kcm.fontsAASettings
                        settingName: "hinting"
                        extraEnabledConditions: antiAliasingCheckBox.checked && !kcm.fontsAASettings.isAaImmutable
                    }
                }
                ContextualHelpButton {
                    toolTipText: xi18nc("@info:tooltip Hinting", "Hinting is a technique in which hints embedded in a font are used to enhance the rendering quality especially at small sizes. Stronger hinting generally leads to sharper edges but the small letters will less closely resemble their shape at big sizes.")
                }
            }

            RowLayout {
                Layout.preferredWidth: formLayout.maxImplicitWidth

                QtControls.CheckBox {
                    id: dpiCheckBox
                    checked: kcm.fontsAASettings.dpi !== 0
                    text: i18n("Force font DPI:")
                    onClicked: {
                        kcm.fontsAASettings.dpi = checked ? dpiSpinBox.value : 0
                        dpiTwiddledMessage.visible = checked
                    }

                    // dpiSpinBox will set forceFontDPI or forceFontDPIWayland,
                    // so only one SettingStateBinding will be activated at a time.
                    KCM.SettingStateBinding {
                        configObject: kcm.fontsAASettings
                        settingName: "forceFontDPIWayland"
                        extraEnabledConditions: antiAliasingCheckBox.checked && !kcm.fontsAASettings.isAaImmutable
                    }
                    KCM.SettingStateBinding {
                        configObject: kcm.fontsAASettings
                        settingName: "forceFontDPI"
                        extraEnabledConditions: antiAliasingCheckBox.checked && !kcm.fontsAASettings.isAaImmutable
                    }
                }

                QtControls.SpinBox {
                    id: dpiSpinBox
                    editable: true
                    value: kcm.fontsAASettings.dpi !== 0 ? kcm.fontsAASettings.dpi : 96
                    onValueModified: kcm.fontsAASettings.dpi = value
                    to: 999
                    from: 1

                    // dpiSpinBox will set forceFontDPI or forceFontDPIWayland,
                    // so only one SettingStateBinding will be activated at a time.
                    KCM.SettingStateBinding {
                        configObject: kcm.fontsAASettings
                        settingName: "forceFontDPIWayland"
                        extraEnabledConditions: dpiCheckBox.enabled && dpiCheckBox.checked
                    }
                    KCM.SettingStateBinding {
                        configObject: kcm.fontsAASettings
                        settingName: "forceFontDPI"
                        extraEnabledConditions: dpiCheckBox.enabled && dpiCheckBox.checked
                    }
                }
                ContextualHelpButton {
                    toolTipText: xi18nc("@info:tooltip Force fonts DPI", "<para>This option forces a specific DPI value for fonts. It may be useful when the real DPI of the hardware is not detected properly and it is also often misused when poor quality fonts are used that do not look well with DPI values other than 96 or 120 DPI.</para><para>The use of this option is generally discouraged.</para><para>If you are using the <emphasis>X Window System</emphasis>, for selecting the proper DPI value a better option is explicitly configuring it for the whole X server if possible (e.g. DisplaySize in xorg.conf). When fonts do not render properly with the real DPI value better fonts should be used or configuration of font hinting should be checked.</para>")
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
