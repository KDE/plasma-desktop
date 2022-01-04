/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras

ColumnLayout {
    id: root

    property var reason
    property var errorInformation: {}

    readonly property real minimumPreferredWidth: PlasmaCore.Units.gridUnit * 12
    readonly property real minimumPreferredHeight: PlasmaCore.Units.gridUnit * 12

    Layout.minimumWidth: Math.max(buttonLayout.implicitWidth, headerIcon.implicitWidth + PlasmaCore.Units.gridUnit * 2)
    Layout.minimumHeight: headingLayout.implicitHeight + spacing + buttonLayout.implicitHeight + PlasmaCore.Units.gridUnit * 2
    // Same as systray popups
    Layout.preferredWidth: PlasmaCore.Units.gridUnit * 24
    Layout.preferredHeight: PlasmaCore.Units.gridUnit * 24

    spacing: textArea.topPadding

    RowLayout {
        id: headingLayout
        spacing: 0
        Layout.margins: PlasmaCore.Units.gridUnit
        Layout.minimumWidth: headerIcon.implicitWidth
        Layout.minimumHeight: headerIcon.implicitHeight
        Layout.maximumHeight: headerIcon.implicitHeight
        Layout.fillWidth: true
        PlasmaCore.IconItem {
            id: headerIcon
            implicitWidth: PlasmaCore.Units.iconSizes.huge
            implicitHeight: PlasmaCore.Units.iconSizes.huge
            source: "dialog-error"
        }
        PlasmaExtras.Heading {
            id: heading
            // Descent is equal to the amount of space above and below capital letters.
            // Add descent to the sides to make the spacing around Latin text look more even.
            leftPadding: headingFontMetrics.descent
            rightPadding: headingFontMetrics.descent
            text: root.errorInformation && root.errorInformation.appletName ?
                i18nd("plasma_shell_org.kde.plasma.desktop", "Sorry! There was an error loading %1.", root.errorInformation.appletName)
                // This is just to suppress warnings. Users should never see this.
                : "No error information."
            level: 2
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            Layout.fillWidth: true
            Layout.maximumHeight: headerIcon.implicitHeight
            FontMetrics {
                id: headingFontMetrics
                font: heading.font
            }
        }
    }

    RowLayout {
        id: buttonLayout
        spacing: parent.spacing
        Layout.alignment: Qt.AlignCenter
        PC3.Button {
            id: copyButton
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Copy to Clipboard")
            icon.name: "edit-copy"
            onClicked: {
                textArea.selectAll()
                textArea.copy()
                textArea.deselect()
            }
        }
        Loader {
            id: compactContentLoader
            active: root.width < root.minimumPreferredWidth
                || root.height < root.minimumPreferredHeight
            visible: active
            sourceComponent: PC3.Button {
                icon.name: "window-new"
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "View Error Details…")
                checked: dialog.visible
                onClicked: dialog.visible = !dialog.visible
                QQC2.ApplicationWindow {
                    id: dialog
                    flags: Qt.Dialog | Qt.WindowStaysOnTopHint | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint
                    minimumWidth: dialogFontMetrics.height * 12
                        + dialogTextArea.leftPadding + dialogTextArea.rightPadding
                    minimumHeight: dialogFontMetrics.height * 12
                        + dialogTextArea.topPadding + dialogTextArea.bottomPadding
                    width: PlasmaCore.Units.gridUnit * 24
                    height: PlasmaCore.Units.gridUnit * 24
                    color: palette.base
                    QQC2.ScrollView {
                        id: dialogScrollView
                        anchors.fill: parent
                        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff
                        QQC2.TextArea {
                            id: dialogTextArea
                            // HACK: silence binding loop warnings.
                            // contentWidth seems to be causing the binding loop,
                            // but contentWidth is read-only and we have no control
                            // over how it is calculated.
                            implicitWidth: 0
                            wrapMode: TextEdit.Wrap
                            text: textArea.text
                            font.family: "monospace"
                            readOnly: true
                            selectByMouse: true
                            background: null
                            FontMetrics {
                                id: dialogFontMetrics
                                font: dialogTextArea.font
                            }
                        }
                        background: null
                    }
                }
            }
        }
    }

    PC3.ScrollView {
        id: fullContentView
        // Not handled by a Loader because we need
        // TextEdit::copy() to copy to clipboard.
        visible: !compactContentLoader.visible
        Layout.fillHeight: true
        Layout.fillWidth: true
        PC3.ScrollBar.horizontal.policy: PC3.ScrollBar.AlwaysOff
        PC3.TextArea {
            id: textArea
            // HACK: silence binding loop warnings.
            // contentWidth seems to be causing the binding loop,
            // but contentWidth is read-only and we have no control
            // over how it is calculated.
            implicitWidth: 0
            wrapMode: TextEdit.Wrap
            text: root.errorInformation && root.errorInformation.errors ?
                root.errorInformation.errors.join("\n\n")
                // This is just to suppress warnings. Users should never see this.
                : "No error information."
            font.family: "monospace"
            readOnly: true
            selectByMouse: true
        }
    }
}
