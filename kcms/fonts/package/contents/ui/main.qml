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
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Dialogs 1.2 as QtDialogs
import org.kde.kquickcontrolsaddons 2.0
import QtQuick.Controls.Private 1.0
//We need units from it
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kcm 1.0

ColumnLayout {
    id: root
    implicitWidth: units.gridUnit * 50
    implicitHeight: units.gridUnit * 50

    ConfigModule.quickHelp: i18n("Fonts")

    SystemPalette {id: syspal}

    QtControls.ScrollView {
        id: scrollView
        Layout.fillWidth: true
        Layout.preferredHeight:  view.contentItem.childrenRect.height
        anchors.top: parent.top

        ListView {
            id: view
            model: kcm.fontsModel
            anchors.fill: parent
            spacing: 10

            delegate: RowLayout {
                id: row
                width: parent.width

                QtControls.Label {
                    id: fontCategory
                    text: model.display
                    Layout.minimumWidth: units.gridUnit * 5
                    horizontalAlignment: Text.AlignRight
                }

                //FIXME
                //QtControls.TextField is buggy with some fonts and it loses
                //its border. So we are using a Label inside to a Rectangle
                //in order to overide that issue.
                Rectangle {
                    id: fakeBorder
                    Layout.minimumHeight: units.gridUnit * 2
                    border.color: syspal.mid
                    color: syspal.window
                    Layout.fillWidth: true
                    anchors {
                        top: parent.top
                        bottom: parent.bottom
                        left: fontCategory.right
                        right: fontButton.left
                        rightMargin: 5
                        leftMargin: 10
                    }

                }
                QtControls.Label {
                    text: model.font.family + " " + model.font.pointSize
                    font: model.font
                    anchors {
                        left: fakeBorder.left
                        leftMargin: 5
                        horizontalCenter: fakeBorder.horizontalCenter
                    }
                }

                QtControls.Button {
                    id: fontButton
                    text: i18n("Choose...")
                    implicitHeight: fakeBorder.height
                    anchors.right: parent.right
                    onClicked: {
                        fontDialog.adjustAllFonts = false;
                        fontDialog.currentIndex = index;
                        fontDialog.currentFont = model.font;
                        fontDialog.open()
                    }
                }
            }
        }
    }

    QtControls.Button {
        id: adjustAllFontsButton
        text: i18n("Adjust All Fonts...")

        anchors {
            top: scrollView.bottom
            topMargin: 20
            right: parent.right
        }

        onClicked: {
            fontDialog.adjustAllFonts = true;
            fontDialog.open()
        }
    }


    GridLayout {
        id: aaSettingsRow
        columns: 3
        Layout.alignment : Qt.AlignLeft
        anchors.top: adjustAllFontsButton.bottom
        Layout.maximumWidth: 300

        QtControls.Label {
            text: i18n("Use anti-aliasing:")
            font.pixelSize: 14
            anchors {
                right: antiAliasingComboBox.left
                rightMargin: 20
            }
        }

        QtControls.ComboBox {
            id: antiAliasingComboBox
            //The ComboBox width doesn't scale based on the length or the
            //currentText.
            implicitWidth: parent.width / 1.9

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
            implicitWidth: antiAliasingComboBox.width
            maximumValue: 1000
            minimumValue: 96
        }
    }

    QtDialogs.Dialog {
        id: advancedDialog
        title: i18n("Configure Anti-Alias Settings")
        standardButtons: QtDialogs.StandardButton.Ok | QtDialogs.StandardButton.Cancel
        width: units.gridUnit * 17

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
                    suffix: " pt"
                    enabled: excludeCheckBox.checked
                }

                QtControls.Label {
                    text: i18n("to")
                }

                QtControls.SpinBox {
                    id: excludeToSpinBox
                    stepSize: 1
                    suffix: " pt"
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

